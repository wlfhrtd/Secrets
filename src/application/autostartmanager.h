#ifndef AUTOSTARTMANAGER_H
#define AUTOSTARTMANAGER_H

#pragma once

#include <QString>

class AutostartManager
{
public:
    static bool isEnabled();
    static void setEnabled(bool enabled, bool startMinimized);

private:
    static QString appName();
};

#endif // AUTOSTARTMANAGER_H
