#include "autostartmanager.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

QString AutostartManager::appName()
{
    return QCoreApplication::applicationName();
}

#ifdef Q_OS_LINUX

void AutostartManager::setEnabled(bool enabled, bool startMinimized)
{
    // Linux (XDG autostart)
    // ~/.config/autostart/secrets.desktop
    QString autostartDir = QStandardPaths::writableLocation(
                               QStandardPaths::ConfigLocation) + "/autostart";

    QDir().mkpath(autostartDir);

    QString filePath = autostartDir + "/" + appName().toLower() + ".desktop";

    if (!enabled)
    {
        QFile::remove(filePath);

        return;
    }

    QString execLine = "\"" + QCoreApplication::applicationFilePath() + "\"";

    if (startMinimized)
    {
        execLine += " --minimized";
    }

    QString content =
        "[Desktop Entry]\n"
        "Type=Application\n"
        "Name=" + appName() + "\n"
        "Exec=" + execLine + "\n"
        "X-GNOME-Autostart-enabled=true\n"
        "Terminal=false\n"
        "Version=1.0\n";

    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        f.write(content.toUtf8());
    }
}

bool AutostartManager::isEnabled()
{
    QString autostartDir = QStandardPaths::writableLocation(
                               QStandardPaths::ConfigLocation) + "/autostart";

    QString filePath = autostartDir + "/" + appName().toLower() + ".desktop";

    return QFile::exists(filePath);
}
#endif

#ifdef Q_OS_WIN

static const wchar_t *RUN_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";

void AutostartManager::setEnabled(bool enabled, bool startMinimized)
{
    HKEY hKey;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
    {
        return;
    }

    std::wstring applicationName = appName().toStdWString();

    if (enabled) {
        // ancient Windows Registry parser doesn't work with "/" in path
        // use QDir::toNativeSeparators and "\" instead
        QString exePath = QDir::toNativeSeparators(
            QCoreApplication::applicationFilePath());
        QString value = "\"" + exePath + "\"";

        if (startMinimized)
        {
            value += " --minimized";
        }

        RegSetValueExW(
            hKey,
            applicationName.c_str(),
            0,
            REG_SZ,
            (const BYTE*)value.toStdWString().c_str(),
            (value.size() + 1) * sizeof(wchar_t)
            );
    } else {
        RegDeleteValueW(hKey, applicationName.c_str());
    }

    RegCloseKey(hKey);
}

bool AutostartManager::isEnabled()
{
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
        return false;
    }

    wchar_t buffer[512];
    DWORD size = sizeof(buffer);

    std::wstring applicationName = appName().toStdWString();

    LONG res = RegQueryValueExW(
        hKey,
        applicationName.c_str(),
        nullptr,
        nullptr,
        (LPBYTE)buffer,
        &size
        );

    RegCloseKey(hKey);

    return res == ERROR_SUCCESS;
}

#endif