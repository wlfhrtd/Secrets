#ifndef SYSTEMCONTEXT_H
#define SYSTEMCONTEXT_H

#pragma once

#include <QString>
#include <QPalette>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

struct SystemTheme
{
    QString styleName;
    QPalette palette;
};

struct SystemContext
{
    inline static SystemTheme theme;

#ifdef Q_OS_WIN

    static bool isWindowsDarkTheme()
    {
        QSettings settings(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
            QSettings::NativeFormat);

        return settings.value("AppsUseLightTheme", 1).toInt() == 0;
    }
#endif

    // for potential ideas in future
    // inline static QString osName;
    // inline static QString osVersion;
    // inline static QString appDataPath;
    // inline static QString documentsPath;
};

#endif // SYSTEMCONTEXT_H