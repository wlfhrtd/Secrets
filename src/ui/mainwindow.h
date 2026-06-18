#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui/theme/palettemode.h"
#include "ui/theme/themestyle.h"
#pragma once

#include <memory>
#include <QMainWindow>
#include <QModelIndex>

#include <application/appsettings.h>
#include <infrastructure/settingsstorage.h>

class QLabel;
class QSystemTrayIcon;

class VaultService;
class VaultSession;
class TreeModel;
class TreeFilterProxyModel;
class LanguageManager;
class IconProvider;
class StylesProvider;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

enum class RevealState
{
    Hidden,
    Countdown,
    Visible
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::MainWindow* ui;

    TreeModel* model;
    TreeFilterProxyModel* proxyModel;

    std::unique_ptr<VaultService> m_vault;

    std::unique_ptr<VaultSession> m_session;
    bool maybeSaveVault();
    bool m_exiting = false;

    // delegates
    void onNewVault();
    void onOpenVault();
    void onSaveVault();
    void onSaveVaultAs();
    void onExportXml();
    bool saveVaultTo(const QString& path);
    void onNewFolder();
    void onNewEntry();
    void onRename();
    void onDelete();
    void onCut();
    void onCopy();
    void onPaste();
    void onChangeMasterPassword();
    void exitApplication();
    void onSettings();

    // tree view hooks
    // previous - m_currentSourceIndex; new/current - current
    bool m_restoringCurrentIndex = false; // recursion guard
    void onCurrentChanged(const QModelIndex& current, const QModelIndex& previous);

    // right panel - editor
    bool m_editMode = false;
    bool m_noteModified = false;

    // note dirty
    bool maybeSaveNote();

    void setupActions();

    void setupMenuBar();

    void adjustInitialLayout();

    // note tool bar
    void setupNoteToolBar();

    void updateNoteToolBarState();

    // note tool bar - edit/save button
    void closeCurrentNote();
    void resetEditState();
    void onEditClicked();
    QAction* m_noteEditAction = nullptr;
    QModelIndex m_currentSourceIndex;
    bool commitEditorChanges();
    // note tool bar - reveal/hide button
    QString hiddenContent() const;
    void hideCurrentNote();
    void resetRevealState();
    void onRevealClicked();
    QAction* m_noteRevealAction = nullptr;
    QTimer* m_revealTimer = nullptr;
    QLabel* m_revealCountdownLabel = nullptr;
    RevealState m_revealState = RevealState::Hidden;
    int m_revealSecondsLeft = 0;
    QString m_revealedContent;
    // note tool bar - copy selected button
    QAction* m_noteCopySelectedAction = nullptr;
    void onCopySelectedClicked();

    void setupTreeToolBar();

    // status bar
    void setupStatusBar();
    void updateVaultLabel();
    QLabel* m_vaultLabel = nullptr;

    // app settings
    AppSettings m_settings;
    SettingsStorage m_settingsStorage;

    // theme
    ThemeStyle m_themeStyle;
    QPalette m_themePalette;
    std::unique_ptr<StylesProvider> m_stylesProvider;
    void applyTheme();
    void configureTheme();
    void repolishWidgets();
    void fixSplitterGlueing();

    // language
    std::unique_ptr<LanguageManager> m_languageManager;
    void changeEvent(QEvent* event) override;
    void retranslateCustomUI();

    // tray
    QSystemTrayIcon* trayIcon = nullptr;
    QMenu* trayMenu = nullptr;
    QAction* m_trayShowAction = nullptr;
    QAction* m_trayHideAction = nullptr;
    QAction* m_trayQuitAction = nullptr;
    void setupTray();
    void setTrayEnabled(bool enabled);
    void applyTraySettings();

    // tree context menu
    void setupTreeContextMenu();
    void showTreeContextMenu(const QPoint& pos);

    //icons
    QColor m_windowIconColor;
    QColor m_uiIconColor;
    QColor m_trayIconColor;
    QColor m_trayMenuIconColor;
    std::unique_ptr<IconProvider> m_iconProvider;
    void updateWindowIcon();
    void updateUiIcons();
    void updateTrayIcons();
};
#endif // MAINWINDOW_H