#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#pragma once

#include <QDialog>
#include <application/appsettings.h>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const AppSettings& settings,
                            QWidget* parent = nullptr);
    ~SettingsDialog() override;

    AppSettings settings() const;

private:
    Ui::SettingsDialog* ui = nullptr;
    AppSettings m_settings;
};

#endif