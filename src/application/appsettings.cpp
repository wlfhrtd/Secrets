#include "appsettings.h"

void AppSettings::normalize()
{
    if (!trayEnabled)
    {
        closeToTray = false;
    }

    if (!autostart)
    {
        startMinimized = false;
    }
}