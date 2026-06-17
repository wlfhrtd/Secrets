#include "languagemanager.h"
#include <QLibraryInfo>
#include <qapplication.h>


void LanguageManager::apply(LanguageMode mode)
{
    qApp->removeTranslator(&m_translator);
    qApp->removeTranslator(&m_qtTranslator);

    switch (mode)
    {
    case LanguageMode::English:
        break;

    case LanguageMode::Russian:
    {
        m_translator.load(":/translations/secrets_ru.qm");

        qApp->installTranslator(&m_translator);

        m_qtTranslator.load(
            QLocale("ru"),
            "qtbase",
            "_",
            QLibraryInfo::path(QLibraryInfo::TranslationsPath));

        qApp->installTranslator(&m_qtTranslator);

        break;
    }
    }
}