#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#pragma once

#include "ui/theme/thememode.h"
#include <QColor>
#include <QPalette>


class ThemeManager
{
public:
    void apply(ThemeMode mode);

    QColor uiIconColor() const;
    QColor systemIconColor() const;
private:
    void applySystemTheme();
    void applyLightTheme();
    void applyDarkTheme();

    QColor m_uiIconColor;
    QColor m_systemIconColor;

    QPalette createFusionLightPalette() const;
    QPalette createFusionDarkPalette() const;
};

#endif // THEMEMANAGER_H
