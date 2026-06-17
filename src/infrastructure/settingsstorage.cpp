#include "settingsstorage.h"

#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QXmlStreamReader>

namespace
{
QString themeToString(ThemeMode theme)
{
    switch (theme)
    {
    case ThemeMode::System:
        return "system";

    case ThemeMode::Light:
        return "light";

    case ThemeMode::Dark:
        return "dark";
    }

    return "system";
}

ThemeMode parseTheme(const QString& value)
{
    if (value.compare("light", Qt::CaseInsensitive) == 0)
    {
        return ThemeMode::Light;
    }

    if (value.compare("dark", Qt::CaseInsensitive) == 0)
    {
        return ThemeMode::Dark;
    }

    return ThemeMode::System;
}

QString languageToString(LanguageMode language)
{
    switch (language)
    {
    case LanguageMode::English:
        return "en";

    case LanguageMode::Russian:
        return "ru";
    }

    return "en";
}

LanguageMode parseLanguage(const QString& value)
{
    if (value.compare("ru", Qt::CaseInsensitive) == 0)
    {
        return LanguageMode::Russian;
    }

    return LanguageMode::English;
}
}

bool SettingsStorage::save(const QString& path, const AppSettings& settings)
{
    QByteArray xml;
    QXmlStreamWriter writer(&xml);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("settings");

    writer.writeTextElement("theme", themeToString(settings.theme));
    writer.writeTextElement("language", languageToString(settings.language));

    writer.writeTextElement("trayEnabled",
                            settings.trayEnabled ? "true" : "false");

    writer.writeTextElement("closeToTray",
                            settings.closeToTray ? "true" : "false");

    writer.writeTextElement("revealTimeoutSeconds",
                            QString::number(settings.revealTimeoutSeconds));

    writer.writeTextElement("autostart",
                            settings.autostart ? "true" : "false");

    writer.writeTextElement("startMinimized",
                            settings.startMinimized ? "true" : "false");

    writer.writeEndElement();
    writer.writeEndDocument();

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    return file.write(xml) == xml.size();
}

bool SettingsStorage::load(const QString& path, AppSettings& settings)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    // reset to defaults
    settings = AppSettings{};

    QByteArray data = file.readAll();
    QXmlStreamReader reader(data);

    while (!reader.atEnd())
    {
        reader.readNext();

        if (!reader.isStartElement())
        {
            continue;
        }

        const auto name = reader.name();

        if (name == "theme")
        {
            settings.theme = parseTheme(reader.readElementText());
        }
        else if (name == "language")
        {
            settings.language = parseLanguage(reader.readElementText());
        }
        else if (name == "trayEnabled")
        {
            settings.trayEnabled =
                (reader.readElementText().compare("true", Qt::CaseInsensitive) == 0);
        }
        else if (name == "closeToTray")
        {
            settings.closeToTray =
                (reader.readElementText().compare("true", Qt::CaseInsensitive) == 0);
        }
        else if (name == "revealTimeoutSeconds")
        {
            bool ok = false;
            int v = reader.readElementText().toInt(&ok);
            if (ok)
            {
                settings.revealTimeoutSeconds = v;
            }
        }
        else if (name == "autostart")
        {
            settings.autostart =
                (reader.readElementText().compare("true", Qt::CaseInsensitive) == 0);
        }
        else if (name == "startMinimized")
        {
            settings.startMinimized =
                (reader.readElementText().compare("true", Qt::CaseInsensitive) == 0);
        }
    }

    if (reader.hasError())
    {
        return false;
    }

    settings.normalize();

    return true;
}

QString SettingsStorage::settingsFile()
{
    // should become ~/.config/Secrets
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);

    QDir dir(base);
    dir.mkpath("Config");

    return dir.filePath("Config/settings.xml");
}