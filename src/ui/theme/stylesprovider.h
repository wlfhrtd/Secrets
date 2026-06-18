#ifndef STYLESPROVIDER_H
#define STYLESPROVIDER_H

#pragma once

#include "ui/theme/palettemode.h"
#include "ui/theme/iconcolor.h"
#include "ui/theme/themestyle.h"

#include <QPalette>
#include <QStyle>


class StylesProvider
{
public:
    QStyle* style(ThemeStyle style) const;
    QPalette palette(PaletteMode mode) const;
    QColor iconColor(IconColor color) const;

private:
    QPalette fusionLightPalette() const;
    QPalette fusionDarkPalette() const;

};

#endif // STYLESPROVIDER_H
