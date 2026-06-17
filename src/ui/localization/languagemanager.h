#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#pragma once

#include "ui/localization/languagemode.h"
#include <QTranslator>


class LanguageManager
{
public:
    void apply(LanguageMode mode);

private:
    QTranslator m_translator;
    QTranslator m_qtTranslator;
};

#endif // LANGUAGEMANAGER_H
