#ifndef SETTINGSSTORAGE_H
#define SETTINGSSTORAGE_H

#pragma once

#include <QString>
#include "application/appsettings.h"

class SettingsStorage
{
public:
    bool load(const QString& path, AppSettings& settings);
    bool save(const QString& path, const AppSettings& settings);

    static QString settingsFile();
};

#endif // SETTINGSSTORAGE_H
