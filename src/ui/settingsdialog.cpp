#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QPushButton>

SettingsDialog::SettingsDialog(
    const AppSettings& settings,
    QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::SettingsDialog)
    , m_settings(settings)
{
    ui->setupUi(this);

    connect(
        ui->buttonBox,
        &QDialogButtonBox::accepted,
        this,
        &QDialog::accept);

    connect(
        ui->buttonBox,
        &QDialogButtonBox::rejected,
        this,
        &QDialog::reject);

    // general tab
    ui->themeComboBox->setCurrentIndex(
        static_cast<int>(settings.theme));

    ui->languageComboBox->setCurrentIndex(
        static_cast<int>(settings.language));

    // tray & startup tab
    ui->enableTrayCheckBox->setChecked(settings.trayEnabled);
    ui->closeToTrayCheckBox->setChecked(settings.closeToTray);

    ui->closeToTrayCheckBox->setEnabled(settings.trayEnabled);

    connect(ui->enableTrayCheckBox, &QCheckBox::toggled,
            this, [this](bool enabled)
            {
                ui->closeToTrayCheckBox->setEnabled(enabled);

                if (!enabled)
                    ui->closeToTrayCheckBox->setChecked(false);
            });

    ui->startWithSystemCheckBox->setChecked(settings.autostart);
    ui->startMinimizedCheckBox->setChecked(settings.startMinimized);

    ui->startMinimizedCheckBox->setEnabled(settings.autostart);

    connect(ui->startWithSystemCheckBox, &QCheckBox::toggled,
            this, [this](bool enabled)
            {
                ui->startMinimizedCheckBox->setEnabled(enabled);

                if (!enabled)
                    ui->startMinimizedCheckBox->setChecked(false);
            });

    // security tab
    ui->spinBox->setValue(settings.revealTimeoutSeconds);

    // disable closeToTray if tray disabled
    if (!settings.trayEnabled)
    {
        ui->closeToTrayCheckBox->setEnabled(false);
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("OK"));
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));
}

AppSettings SettingsDialog::settings() const
{
    AppSettings result = m_settings;

    result.theme = static_cast<ThemeMode>(
        ui->themeComboBox->currentIndex());

    result.language = static_cast<LanguageMode>(
        ui->languageComboBox->currentIndex());

    result.trayEnabled = ui->enableTrayCheckBox->isChecked();

    result.closeToTray = ui->closeToTrayCheckBox->isChecked();

    result.autostart = ui->startWithSystemCheckBox->isChecked();

    result.startMinimized = ui->startMinimizedCheckBox->isChecked();

    result.revealTimeoutSeconds = ui->spinBox->value();

    result.normalize();

    return result;
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}