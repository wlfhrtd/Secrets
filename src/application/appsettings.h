#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#pragma once

#include "ui/localization/languagemode.h"
#include "ui/theme/thememode.h"


struct AppSettings
{
    ThemeMode theme = ThemeMode::System;
    LanguageMode language = LanguageMode::English;

    bool trayEnabled = true;
    bool closeToTray = true;

    int revealTimeoutSeconds = 10;

    bool autostart = false;
    bool startMinimized = false;

    void normalize();
};

#endif // APPSETTINGS_H
