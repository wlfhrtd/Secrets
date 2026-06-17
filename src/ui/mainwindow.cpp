#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"

#include <QTimer>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QClipboard>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QLabel>
#include <model/treemodel.h>
#include <model/treefilterproxymodel.h>
#include <infrastructure/xmlstorage.h>
#include <application/autostartmanager.h>
#include <application/vaultservice.h>
#include <application/vaultsession.h>
#include <qabstractbutton.h>
#include <shared/systemcontext.h>
#include <shared/applicationcontext.h>
#include "ui/localization/languagemanager.h"
#include "ui/icons/iconprovider.h"
#include "ui/theme/thememanager.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , model(new TreeModel(this))
    , proxyModel(new TreeFilterProxyModel(this))
{
    ui->setupUi(this);

    adjustInitialLayout();

    // init
    m_settingsStorage.load(SettingsStorage::settingsFile(), m_settings);

    m_languageManager = std::make_unique<LanguageManager>();

    m_iconProvider = std::make_unique<IconProvider>();

    m_themeManager = std::make_unique<ThemeManager>();

    m_vault = std::make_unique<VaultService>(model);

    m_session = std::make_unique<VaultSession>();

    setupActions();
    setupMenuBar();
    setupTreeToolBar();
    setupTreeContextMenu();

    setupNoteToolBar();
    updateNoteToolBarState();

    setupStatusBar();

    // UI refresh
    applyTheme();
    m_languageManager->apply(m_settings.language);

    // tray is independent infra
    // but should be created after theme
    setupTray();
    applyTraySettings();

    // we decide when to edit (F2, Menu-Rename) manually
    ui->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // model → proxy
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(0);

    // show proxy not source model
    ui->treeView->setModel(proxyModel);

    // sort
    proxyModel->setDynamicSortFilter(true);
    ui->treeView->setSortingEnabled(true);
    proxyModel->sort(0);

    // resetting current index with click in blank
    ui->treeView->viewport()->installEventFilter(this);

    // fast filter (title only)
    connect(ui->lineEdit,
            &QLineEdit::textChanged,
            this,
            [this](const QString& text)
            {
                proxyModel->setFilterRegularExpression(text);

                if (!text.isEmpty())
                    ui->treeView->expandAll();
            });

    // selection in tree view - current changed
    connect(
        ui->treeView->selectionModel(),
        &QItemSelectionModel::currentChanged,
        this,
        &MainWindow::onCurrentChanged);

    // note is modified
    connect(
        ui->plainTextEdit,
        &QPlainTextEdit::textChanged,
        this,
        [this]()
        {
            // catch only if !read-only - not any textChanged events
            if (m_editMode)
            {
                m_noteModified = true;
            }
        });

    // vault is modified; reason - model modified
    connect(
        model,
        &TreeModel::modified,
        this,
        [this]()
        {
            m_session->setDirty(true);
        });
}

void MainWindow::onSettings()
{
    SettingsDialog dlg(m_settings, this);

    if (dlg.exec() != QDialog::Accepted)
    {
        return;
    }

    AppSettings result = dlg.settings();

    result.normalize();

    const int oldRevealTimeout = m_settings.revealTimeoutSeconds;

    m_settings = result;

    AutostartManager::setEnabled(m_settings.autostart, m_settings.startMinimized);

    applyTheme();
    m_languageManager->apply(m_settings.language);

    setTrayEnabled(m_settings.trayEnabled);

    applyTraySettings();

    // calculate remaining time if user changed
    // Reveal timeout during Countdown
    if (m_revealState == RevealState::Countdown
        && oldRevealTimeout != m_settings.revealTimeoutSeconds)
    {
        m_revealSecondsLeft = std::min(
            m_revealSecondsLeft,
            m_settings.revealTimeoutSeconds);
    }

    m_settingsStorage.save(SettingsStorage::settingsFile(), m_settings);
}

bool MainWindow::maybeSaveVault()
{
    if (!m_session->isDirty())
    {
        return true;
    }

    QMessageBox box(this);
    box.setWindowTitle(tr("Unsaved vault changes"));
    box.setText(tr("Save changes to the vault?"));
    box.setStandardButtons(
        QMessageBox::Save
        | QMessageBox::Discard
        | QMessageBox::Cancel);

    box.button(QMessageBox::Save)->setText(tr("Save"));
    box.button(QMessageBox::Discard)->setText(tr("Discard"));
    box.button(QMessageBox::Cancel)->setText(tr("Cancel"));

    switch (box.exec())
    {
    case QMessageBox::Save:
        onSaveVault();

        return !m_session->isDirty();

    case QMessageBox::Discard:
        return true;

    case QMessageBox::Cancel:

    default:
        return false;
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!maybeSaveNote())
    {
        m_exiting = false;

        event->ignore();

        return;
    }

    if (!maybeSaveVault())
    {
        m_exiting = false;

        event->ignore();

        return;
    }

    if (!m_exiting
        && m_settings.closeToTray
        && trayIcon
        && trayIcon->isVisible())
    {
        event->ignore();

        hide();

        return;
    }

    event->accept();
}

// note tool bar - copy selected button
void MainWindow::onCopySelectedClicked()
{
    if (!m_currentSourceIndex.isValid())
    {
        return;
    }

    // allowed state
    const bool canCopy =
        m_editMode
        || m_revealState == RevealState::Visible
        || m_revealState == RevealState::Countdown;

    if (!canCopy)
    {
        return;
    }

    const QTextCursor cursor = ui->plainTextEdit->textCursor();

    if (!cursor.hasSelection())
    {
        return;
    }

    const QString text = cursor.selectedText();

    if (text.isEmpty())
    {
        return;
    }

    QClipboard* clipboard = QGuiApplication::clipboard();

    clipboard->setText(text);
}

void MainWindow::onCut()
{
    QModelIndex current = ui->treeView->currentIndex();

    QModelIndex source = proxyModel->mapToSource(current);

    model->cutNode(source);

    return;
}

void MainWindow::resetEditState()
{
    m_editMode = false;
    m_noteModified = false;

    ui->plainTextEdit->setReadOnly(true);

    updateNoteToolBarState();
}

void MainWindow::closeCurrentNote()
{
    m_currentSourceIndex = QModelIndex();

    resetEditState();

    resetRevealState();

    ui->plainTextEdit->clear();
}

QString MainWindow::hiddenContent() const
{
    if (!m_currentSourceIndex.isValid())
    {
        return {};
    }

    Node* node = model->nodeFromIndex(m_currentSourceIndex);

    if (!node)
    {
        return {};
    }

    if (!node->isEntry())
    {
        return {};
    }

    if (node->content.isEmpty())
    {
        return {};
    }

    return QString(node->content.size(), QChar('*'));
}

// reveal/hide button helper
void MainWindow::hideCurrentNote()
{
    if (!m_currentSourceIndex.isValid())
    {
        return;
    }

    Node* node = model->nodeFromIndex(m_currentSourceIndex);

    if (!node)
    {
        return;
    }

    ui->plainTextEdit->setPlainText(QString(node->content.size(), '*'));
}

// reveal/hide button helper
void MainWindow::resetRevealState()
{
    m_revealTimer->stop();

    m_revealState = RevealState::Hidden;

    m_revealSecondsLeft = 0;

    m_revealedContent.clear();

    updateNoteToolBarState();
}

// note tool bar - reveal/hide button
void MainWindow::onRevealClicked()
{
    if (!m_currentSourceIndex.isValid())
    {
        return;
    }

    switch (m_revealState)
    {
    case RevealState::Hidden:
    {
        // forbid reveal start
        if (m_editMode)
        {
            return;
        }

        Node* node = model->nodeFromIndex(m_currentSourceIndex);

        if (!node || !node->isEntry())
        {
            return;
        }

        m_revealedContent = node->content;

        ui->plainTextEdit->setPlainText(m_revealedContent);

        m_revealState = RevealState::Countdown;

        m_revealSecondsLeft = m_settings.revealTimeoutSeconds;

        m_revealTimer->start();

        break;
    }

    case RevealState::Countdown:
    {
        // forbid reveal start
        if (m_editMode)
        {
            return;
        }

        m_revealTimer->stop();

        m_revealState = RevealState::Visible;

        break;
    }

    // note tool bar - Hide
    case RevealState::Visible:
    {
        if (m_editMode)
        {
            // discard unsaved changes
            resetEditState();
        }

        hideCurrentNote();

        m_revealTimer->stop();

        m_revealSecondsLeft = 0;

        m_revealState = RevealState::Hidden;

        m_revealedContent.clear();

        break;
    }
    }

    updateNoteToolBarState();
}

void MainWindow::onPaste()
{
    QModelIndex current = ui->treeView->currentIndex();

    QModelIndex source = proxyModel->mapToSource(current);

    model->pasteNode(source);
}

void MainWindow::onCopy()
{
    QModelIndex current = ui->treeView->currentIndex();

    QModelIndex source = proxyModel->mapToSource(current);

    model->copyNode(source);

    return;
}

void MainWindow::onNewFolder()
{
    QModelIndex current = ui->treeView->currentIndex();

    QModelIndex source = proxyModel->mapToSource(current);

    model->addFolder(source, tr("New Folder", "object"));

    return;
}

void MainWindow::onNewEntry()
{
    QModelIndex current = ui->treeView->currentIndex();

    QModelIndex source = proxyModel->mapToSource(current);

    model->addEntry(source, tr("New Entry", "object"));

    return;
}

void MainWindow::onRename()
{
    QModelIndex current = ui->treeView->currentIndex();

    if (!current.isValid())
    {
        return;
    }

    QModelIndex source = proxyModel->mapToSource(current);

    Node* node = model->nodeFromIndex(source);

    if (!node)
    {
        return;
    }

    if (node->type == NodeType::Trash)
    {
        return;
    }

    ui->treeView->edit(current);
}

void MainWindow::onDelete()
{
    QModelIndex current = ui->treeView->currentIndex();

    if (!current.isValid())
    {
        return;
    }

    QModelIndex source = proxyModel->mapToSource(current);

    Node* node = model->nodeFromIndex(source);

    if (!node)
    {
        return;
    }

    model->deleteNodeOrMoveToTrash(node);

    closeCurrentNote();
}

void MainWindow::onChangeMasterPassword()
{
    if (m_session->path().isEmpty())
    {
        return;
    }

    bool ok = false;

    QString password = QInputDialog::getText(
        this,
        tr("New Password"),
        tr("Password:"),
        QLineEdit::Password,
        QString(),
        &ok);

    if (!ok || password.isEmpty())
    {
        return;
    }

    m_session->setPassword(password);

    saveVaultTo(m_session->path());

    QMessageBox::information(
        this,
        tr("Password"),
        tr("Master password changed."));
}

// reset current index when click in blank
bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    // click in blank
    if (obj == ui->treeView->viewport()
        && event->type() == QEvent::MouseButtonPress)
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);

        QModelIndex index = ui->treeView->indexAt(mouseEvent->pos());

        if (!index.isValid())
        {
            if (!maybeSaveNote())
            {
                return true;
            }

            // reset note state
            closeCurrentNote();

            ui->treeView->clearSelection();

            ui->treeView->setCurrentIndex(QModelIndex());
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

bool MainWindow::commitEditorChanges()
{
    if (!m_currentSourceIndex.isValid())
    {
        return false;
    }

    Node* node = model->nodeFromIndex(m_currentSourceIndex);

    if (!node)
    {
        return false;
    }

    if (!node->isEntry())
    {
        closeCurrentNote();

        return false;
    }

    return model->setContent(
        m_currentSourceIndex, ui->plainTextEdit->toPlainText());
}

// when current (not selection) in tree changed
void MainWindow::onCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    Q_UNUSED(previous);

    // return user to previous node if pressed Cancel for unsaved changes
    // based on m_restoringCurrentIndex recursion guard
    // and m_currentSourceIndex (instead of "previous" intentionally)
    if (m_restoringCurrentIndex)
    {
        return;
    }

    if (!maybeSaveNote())
    {
        QModelIndex oldProxyIndex = proxyModel->mapFromSource(m_currentSourceIndex);

        // hacking event loop to rollback after selectionChanged
        // which happens after currentChanged
        if (oldProxyIndex.isValid())
        {
            m_restoringCurrentIndex = true;

            QTimer::singleShot(
                0,
                this,
                [this, oldProxyIndex]()
                {
                    ui->treeView->setCurrentIndex(oldProxyIndex);

                    m_restoringCurrentIndex = false;
                });
        }

        return;
    }

    resetEditState();

    resetRevealState();

    if (!current.isValid())
    {
        closeCurrentNote();

        return;
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(current);

    Node* node = static_cast<Node*>(sourceIndex.internalPointer());

    if (!node)
    {
        return;
    }

    m_currentSourceIndex = sourceIndex;

    if (node->isEntry())
    {
        ui->plainTextEdit->setPlainText(hiddenContent());
    }
    else
    {
        ui->plainTextEdit->clear();
    }
}

// note tool bar - Edit/Save button
void MainWindow::onEditClicked()
{
    if (!m_currentSourceIndex.isValid())
    {
        return;
    }

    if (!m_editMode)
    {
        Node* node = model->nodeFromIndex(m_currentSourceIndex);

        if (!node || !node->isEntry())
        {
            return;
        }

        // stop reveal timer
        m_revealTimer->stop();

        m_revealSecondsLeft = 0;

        // force visible state
        m_revealState = RevealState::Visible;

        m_revealedContent = node->content;

        ui->plainTextEdit->setPlainText(node->content);

        ui->plainTextEdit->setReadOnly(false);

        m_editMode = true;

        m_noteModified = false;

        updateNoteToolBarState();

        return;
    }

    // save
    if (m_noteModified)
    {
        if (commitEditorChanges())
        {
            ui->statusbar->showMessage(tr("Note saved"), 5000);
        }
        else
        {
            ui->statusbar->showMessage(tr("Failed to save note"), 5000);
        }
    }

    resetEditState();

    updateNoteToolBarState();
}

bool MainWindow::maybeSaveNote()
{
    if (!m_editMode)
    {
        return true;
    }

    if (!m_noteModified)
    {
        return true;
    }

    QMessageBox box(this);
    box.setWindowTitle(tr("Unsaved changes"));
    box.setText(tr("Save changes to the current note?"));
    box.setStandardButtons(
        QMessageBox::Save
        | QMessageBox::Discard
        | QMessageBox::Cancel);

    box.button(QMessageBox::Save)->setText(tr("Save"));
    box.button(QMessageBox::Discard)->setText(tr("Discard"));
    box.button(QMessageBox::Cancel)->setText(tr("Cancel"));

    switch (box.exec())
    {
    case QMessageBox::Save:
        commitEditorChanges();

        resetEditState();

        return true;

    case QMessageBox::Discard:
        resetEditState();

        return true;

    case QMessageBox::Cancel:

    default:
        return false;
    }
}

void MainWindow::setupMenuBar()
{
    // File
    connect(
        ui->actionNew_Vault,
        &QAction::triggered,
        this,
        &MainWindow::onNewVault);

    connect(
        ui->actionOpen_Vault,
        &QAction::triggered,
        this,
        &MainWindow::onOpenVault);

    connect(
        ui->actionSave_Vault,
        &QAction::triggered,
        this,
        &MainWindow::onSaveVault);

    connect(
        ui->actionSave_Vault_As,
        &QAction::triggered,
        this,
        &MainWindow::onSaveVaultAs);

    connect(
        ui->actionExport_XML,
        &QAction::triggered,
        this,
        &MainWindow::onExportXml);

    connect(
        ui->actionExit,
        &QAction::triggered,
        this,
        &MainWindow::exitApplication);

    // Edit - everything in setupActions()

    // View (some in setupActions())
    connect(
        ui->actionShowTrash,
        &QAction::toggled,
        proxyModel,
        &TreeFilterProxyModel::setShowTrash);

    // Tools
    connect(
        ui->actionSettings,
        &QAction::triggered,
        this,
        &MainWindow::onSettings);

    connect(
        ui->actionChange_Master_Password,
        &QAction::triggered,
        this,
        &MainWindow::onChangeMasterPassword);

    // Help
    connect(
        ui->actionAbout_Secrets,
        &QAction::triggered,
        this,
        [this]()
        {
            QMessageBox::about(
                this,
                tr("About %1")
                    .arg(ApplicationContext::name),
                tr("%1\n\n"
                   "%2\n"
                   "Version %3")
                    .arg(ApplicationContext::name)
                    .arg(ApplicationContext::description)
                    .arg(ApplicationContext::version));
        });

    connect(
        ui->actionAbout_Qt,
        &QAction::triggered,
        qApp,
        &QApplication::aboutQt);
}

void MainWindow::setupTreeToolBar()
{
    ui->treeToolBar->addAction(ui->actionNew_Folder);
    ui->treeToolBar->addAction(ui->actionNew_Entry);

    ui->treeToolBar->addSeparator();

    ui->treeToolBar->addAction(ui->actionRename);
    ui->treeToolBar->addAction(ui->actionDelete);

    ui->treeToolBar->addSeparator();

    ui->treeToolBar->addAction(ui->actionCut);
    ui->treeToolBar->addAction(ui->actionCopy);
    ui->treeToolBar->addAction(ui->actionPaste);

    ui->treeToolBar->addSeparator();

    ui->treeToolBar->addAction(ui->actionCollapse_All);
    ui->treeToolBar->addAction(ui->actionExpand_All);

    QWidget* spacer = new QWidget(this);

    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    ui->treeToolBar->addWidget(spacer);
}

void MainWindow::setupNoteToolBar()
{
    // spacer first, left
    auto *spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->noteToolBar->addWidget(spacer);

    const auto& icons = m_iconProvider->noteToolBarIcons();

    m_noteCopySelectedAction = ui->noteToolBar->addAction(icons.copySelected, QString());
    m_noteRevealAction = ui->noteToolBar->addAction(icons.reveal, QString());

    m_noteCopySelectedAction->setToolTip(tr("Copy selected"));
    m_noteRevealAction->setToolTip(tr("Reveal"));

    m_revealCountdownLabel = new QLabel(this);
    m_revealCountdownLabel->setFixedWidth(30);
    m_revealCountdownLabel->setAlignment(Qt::AlignCenter);
    ui->noteToolBar->addWidget(m_revealCountdownLabel);

    m_noteEditAction = ui->noteToolBar->addAction(icons.edit, QString());
    m_noteEditAction->setToolTip(tr("Edit", "note tool bar edit action"));

    // timer setup
    QWidget* revealWidget = ui->noteToolBar->widgetForAction(m_noteRevealAction);
    revealWidget->setMinimumWidth(30);

    // timer for reveal/hide button
    m_revealTimer = new QTimer(this);
    m_revealTimer->setInterval(1000);
    connect(
        m_revealTimer,
        &QTimer::timeout,
        this,
        [this]()
        {
            if (--m_revealSecondsLeft <= 0)
            {
                m_revealTimer->stop();

                m_revealSecondsLeft = 0;

                hideCurrentNote();

                m_revealedContent.clear();

                m_revealState = RevealState::Hidden;

                updateNoteToolBarState();

                return;
            }

            updateNoteToolBarState();
        });

    connect(
        m_noteCopySelectedAction,
        &QAction::triggered,
        this,
        &MainWindow::onCopySelectedClicked);

    connect(
        m_noteRevealAction,
        &QAction::triggered,
        this,
        &MainWindow::onRevealClicked);

    connect(
        m_noteEditAction,
        &QAction::triggered,
        this,
        &MainWindow::onEditClicked);
}

void MainWindow::updateNoteToolBarState()
{
    const auto& icons = m_iconProvider->noteToolBarIcons();

    // EDIT / SAVE
    if (m_editMode)
    {
        m_noteEditAction->setIcon(icons.save);
        m_noteEditAction->setToolTip(tr("Save"));
    }
    else
    {
        m_noteEditAction->setIcon(icons.edit);
        m_noteEditAction->setToolTip(tr("Edit", "note tool bar edit action"));
    }

    // REVEAL / HIDE / COUNTDOWN
    switch (m_revealState)
    {
    case RevealState::Hidden:
        m_noteRevealAction->setIcon(icons.reveal);

        m_noteRevealAction->setToolTip(tr("Reveal"));

        m_revealCountdownLabel->clear();

        break;

    case RevealState::Countdown:
        m_noteRevealAction->setIcon(icons.stopwatch);

        m_noteRevealAction->setToolTip(tr("Reveal indefinitely"));

        m_revealCountdownLabel->setText(QString::number(m_revealSecondsLeft));

        break;

    case RevealState::Visible:
        m_noteRevealAction->setIcon(icons.hide);

        m_noteRevealAction->setToolTip(tr("Hide"));

        m_revealCountdownLabel->clear();

        break;
    }
}

void MainWindow::onExportXml()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Export XML"),
        QString(),
        tr("XML files (*.xml)"));

    if (path.isEmpty())
    {
        return;
    }

    if (!m_vault->exportXmlFile(path))
    {
        QMessageBox::critical(
            this,
            tr("Export XML"),
            tr("Failed to export XML."));

        return;
    }

    ui->statusbar->showMessage(
        tr("XML exported"),
        5000);
}

void MainWindow::onSaveVaultAs()
{
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Save Vault As"),
        QString(),
        tr("Secrets Vault (*.vault)"));

    if (path.isEmpty())
    {
        return;
    }

    bool ok = false;

    QString password = QInputDialog::getText(
        this,
        tr("Vault Password"),
        tr("Password:"),
        QLineEdit::Password,
        m_session->password(),
        &ok);

    if (!ok)
    {
        return;
    }

    if (m_vault->saveToFile(path, password))
    {
        m_session->setPassword(password);
        m_session->setPath(path);
        m_session->setDirty(false);

        updateVaultLabel();

        ui->statusbar->showMessage(
            tr("Vault saved"),
            5000);
    }
}

void MainWindow::onSaveVault()
{
    if (m_session->path().isEmpty())
    {
        onSaveVaultAs();

        return;
    }

    saveVaultTo(m_session->path());
}

void MainWindow::onOpenVault()
{
    if (!maybeSaveNote())
    {
        return;
    }

    if (!maybeSaveVault())
    {
        return;
    }

    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Open Vault"),
        QString(),
        tr("Secrets Vault (*.vault)"));

    if (path.isEmpty())
    {
        return;
    }

    bool ok = false;

    QString password = QInputDialog::getText(
        this,
        tr("Vault Password"),
        tr("Password:"),
        QLineEdit::Password,
        QString(),
        &ok);

    if (!ok)
    {
        return;
    }

    if (!m_vault->loadFromFile(path, password))
    {
        QMessageBox::critical(
            this,
            tr("Open Vault"),
            tr("Failed to open vault.\n"
               "Wrong password or damaged file."));

        return;
    }

    closeCurrentNote();

    m_session->setPath(path);
    m_session->setPassword(password);
    m_session->setDirty(false);

    updateVaultLabel();

    ui->treeView->expandToDepth(0);

    ui->statusbar->showMessage(tr("Vault loaded"), 5000);
}

void MainWindow::onNewVault()
{
    if (!maybeSaveNote())
    {
        return;
    }

    if (!maybeSaveVault())
    {
        return;
    }

    model->clear();

    closeCurrentNote();

    m_session->clear();

    m_session->setDirty(true);

    updateVaultLabel();

    ui->statusbar->showMessage(tr("New vault created"), 5000);
}

bool MainWindow::saveVaultTo(const QString& path)
{
    if (path.isEmpty())
    {
        return false;
    }

    if (!m_vault->saveToFile(path, m_session->password()))
    {
        QMessageBox::critical(
            this,
            tr("Save Vault"),
            tr("Failed to save vault."));

        return false;
    }

    m_session->setPath(path);
    m_session->setDirty(false);

    updateVaultLabel();

    ui->statusbar->showMessage(tr("Vault saved"), 5000);

    return true;
}

// | regular widgets, showMessage() | permanent widgets |
void MainWindow::setupStatusBar()
{
    m_vaultLabel = new QLabel(this);

    m_vaultLabel->setText(tr("No vault"));

    ui->statusbar->addWidget(m_vaultLabel);
}

void MainWindow::updateVaultLabel()
{
    if (m_session->path().isEmpty())
    {
        m_vaultLabel->setText(tr("No vault"));

        m_vaultLabel->setToolTip("");

        return;
    }

    QFileInfo fileInfo(m_session->path());

    m_vaultLabel->setText(tr("Vault: %1").arg(fileInfo.fileName()));

    m_vaultLabel->setToolTip(QDir::toNativeSeparators(
        fileInfo.absoluteFilePath()));
}

void MainWindow::applyTheme()
{
    m_themeManager->apply(m_settings.theme);

    fixSplitterGlueing();

    repolishWidgets();

    m_iconProvider->setUiColor(m_themeManager->uiIconColor());
    m_iconProvider->setSystemColor(m_themeManager->systemIconColor());

    updateIcons();

    updateNoteToolBarState();
}

void MainWindow::fixSplitterGlueing()
{
    // fix splitter glueing left/right parts together with System theme
    if (m_settings.theme == ThemeMode::System)
    {
        ui->splitter->setStyleSheet("QSplitter::handle { background: transparent; }");
    }
    else
    {
        ui->splitter->setStyleSheet("");
    }
}

void MainWindow::adjustInitialLayout()
{
    ui->splitter->setHandleWidth(6);
    // another splitter hack - start with 1:1 ratio
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 1);
    ui->splitter->setSizes({width() / 2, width() / 2});
}

void MainWindow::repolishWidgets()
{
    for (QWidget* w : qApp->allWidgets())
    {
        w->style()->unpolish(w);
        w->style()->polish(w);
        w->update();
    }
}

void MainWindow::retranslateCustomUI()
{
    m_vaultLabel->setText(tr("No vault"));

    m_trayShowAction->setText(tr("Show"));
    m_trayHideAction->setText(tr("Hide"));
    m_trayQuitAction->setText(tr("Quit"));

    m_noteCopySelectedAction->setToolTip(tr("Copy selected"));
    m_noteRevealAction->setToolTip(tr("Reveal"));
    m_noteEditAction->setToolTip(tr("Edit", "note tool bar edit action"));

    updateNoteToolBarState();
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        retranslateCustomUI();
    }

    QMainWindow::changeEvent(event);
}

void MainWindow::setupTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        return;
    }

    trayIcon = new QSystemTrayIcon(this);

    trayMenu = new QMenu(this);

    QColor trayColor;

#ifdef Q_OS_WIN

    if (SystemContext::isWindowsDarkTheme())
    {
        trayMenu->setPalette(createFusionDarkPalette());

        trayColor = QColor(220, 220, 220);
    }
    else
    {
        trayMenu->setPalette(createFusionLightPalette());

        trayColor = QColor(32, 32, 32);
    }

#else

    trayMenu->setPalette(SystemContext::theme.palette);

    trayColor = SystemContext::theme.palette.color(QPalette::WindowText);

#endif

    trayIcon->setIcon(m_iconProvider->system(IconId::Secrets));

    m_trayShowAction = trayMenu->addAction(m_iconProvider->system(IconId::Maximize), tr("Show"));

    m_trayHideAction = trayMenu->addAction(m_iconProvider->system(IconId::Minimize), tr("Hide"));

    trayMenu->addSeparator();

    m_trayQuitAction = trayMenu->addAction(m_iconProvider->system(IconId::Quit), tr("Quit"));

    connect(
        m_trayShowAction,
        &QAction::triggered,
        this,
        &QWidget::showNormal);

    connect(
        m_trayHideAction,
        &QAction::triggered,
        this,
        &QWidget::hide);

    connect(
        m_trayQuitAction,
        &QAction::triggered,
        this,
        &MainWindow::exitApplication);

    trayIcon->setContextMenu(trayMenu);

    connect(
        trayIcon,
        &QSystemTrayIcon::activated,
        this,
        [this](QSystemTrayIcon::ActivationReason reason)
        {
            if (reason == QSystemTrayIcon::Trigger)
            {
                if (isVisible())
                {
                    hide();
                }
                else
                {
                    showNormal();
                    raise();
                    activateWindow();
                }
            }
        });
}

void MainWindow::applyTraySettings()
{
    if (!trayIcon)
    {
        return;
    }

    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        return;
    }

    if (m_settings.trayEnabled)
    {
        trayIcon->show();
    }
    else
    {
        trayIcon->hide();
    }
}

void MainWindow::setTrayEnabled(bool enabled)
{
    m_settings.trayEnabled = enabled;

    if (!enabled)
    {
        if (trayIcon)
        {
            trayIcon->hide();
        }

        return;
    }

    if (!trayIcon)
    {
        setupTray();
    }

    trayIcon->show();
}

void MainWindow::exitApplication()
{
    m_exiting = true;

    qApp->quit();
}

void MainWindow::setupTreeContextMenu()
{
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(
        ui->treeView,
        &QWidget::customContextMenuRequested,
        this,
        &MainWindow::showTreeContextMenu);
}

void MainWindow::showTreeContextMenu(const QPoint& pos)
{
    QModelIndex index = ui->treeView->indexAt(pos);

    QMenu menu(this);

    menu.addAction(ui->actionNew_Folder);
    menu.addAction(ui->actionNew_Entry);

    if (index.isValid())
    {
        ui->treeView->setCurrentIndex(index);

        menu.addSeparator();

        menu.addAction(ui->actionRename);
        menu.addAction(ui->actionDelete);

        menu.addSeparator();

        menu.addAction(ui->actionCut);
        menu.addAction(ui->actionCopy);
        menu.addAction(ui->actionPaste);
    }

    menu.addSeparator();

    menu.addAction(ui->actionCollapse_All);
    menu.addAction(ui->actionExpand_All);

    menu.exec(ui->treeView->viewport()->mapToGlobal(pos));
}

void MainWindow::setupActions()
{
    // tooltips
    ui->actionNew_Folder->setToolTip(tr("New Folder", "action"));
    ui->actionNew_Entry->setToolTip(tr("New Entry", "action"));

    ui->actionRename->setToolTip(tr("Rename"));
    ui->actionDelete->setToolTip(tr("Delete"));

    ui->actionCut->setToolTip(tr("Cut"));
    ui->actionCopy->setToolTip(tr("Copy"));
    ui->actionPaste->setToolTip(tr("Paste"));

    ui->actionCollapse_All->setToolTip(tr("Collapse All"));
    ui->actionExpand_All->setToolTip(tr("Expand All"));

    // delegates
    connect(
        ui->actionNew_Folder,
        &QAction::triggered,
        this,
        &MainWindow::onNewFolder);

    connect(
        ui->actionNew_Entry,
        &QAction::triggered,
        this,
        &MainWindow::onNewEntry);

    connect(
        ui->actionRename,
        &QAction::triggered,
        this,
        &MainWindow::onRename);

    connect(
        ui->actionDelete,
        &QAction::triggered,
        this,
        &MainWindow::onDelete);

    connect(
        ui->actionCut,
        &QAction::triggered,
        this,
        &MainWindow::onCut);

    connect(
        ui->actionCopy,
        &QAction::triggered,
        this,
        &MainWindow::onCopy);

    connect(
        ui->actionPaste,
        &QAction::triggered,
        this,
        &MainWindow::onPaste);

    connect(
        ui->actionCollapse_All,
        &QAction::triggered,
        this,
        [this]()
        {
            ui->treeView->collapseAll();

            QModelIndex current = ui->treeView->currentIndex();

            if (current.isValid())
                ui->treeView->scrollTo(current);
        });

    connect(
        ui->actionExpand_All,
        &QAction::triggered,
        this,
        [this]()
        {
            ui->treeView->expandAll();
        });
}

void MainWindow::updateIcons()
{
    qApp->setWindowIcon(m_iconProvider->windowIcon());

    m_noteCopySelectedAction->setIcon(m_iconProvider->noteToolBarIcons().copySelected);

    ui->actionNew_Vault->setIcon(m_iconProvider->ui(IconId::NewVault));

    ui->actionOpen_Vault->setIcon(m_iconProvider->ui(IconId::OpenVault));

    ui->actionSave_Vault->setIcon(m_iconProvider->ui(IconId::SaveVault));

    ui->actionSave_Vault_As->setIcon(m_iconProvider->ui(IconId::SaveVaultAs));

    ui->actionExport_XML->setIcon(m_iconProvider->ui(IconId::ExportXml));

    ui->actionExit->setIcon(m_iconProvider->ui(IconId::Exit));

    ui->actionNew_Folder->setIcon(m_iconProvider->ui(IconId::NewFolder));

    ui->actionNew_Entry->setIcon(m_iconProvider->ui(IconId::NewEntry));

    ui->actionRename->setIcon(m_iconProvider->ui(IconId::Rename));

    ui->actionDelete->setIcon(m_iconProvider->ui(IconId::Delete));

    ui->actionCut->setIcon(m_iconProvider->ui(IconId::Cut));

    ui->actionCopy->setIcon(m_iconProvider->ui(IconId::Copy));

    ui->actionPaste->setIcon(m_iconProvider->ui(IconId::Paste));

    ui->actionCollapse_All->setIcon(m_iconProvider->ui(IconId::CollapseAll));

    ui->actionExpand_All->setIcon(m_iconProvider->ui(IconId::ExpandAll));

    ui->actionSettings->setIcon(m_iconProvider->ui(IconId::Settings));

    ui->actionChange_Master_Password->setIcon(m_iconProvider->ui(IconId::ChangeMasterPassword));

    ui->actionAbout_Secrets->setIcon(m_iconProvider->ui(IconId::Secrets));

    ui->actionAbout_Qt->setIcon(m_iconProvider->ui(IconId::AboutQt));
}

MainWindow::~MainWindow()
{
    delete ui;
}