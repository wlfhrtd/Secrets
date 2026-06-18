#include "stylesprovider.h"

#include <QStyleFactory>
#include <shared/systemcontext.h>


QPalette StylesProvider::palette(PaletteMode mode) const
{
    switch (mode)
    {
    case PaletteMode::System:
    {
        return SystemContext::theme.palette;
    }
    case PaletteMode::Light:
    {
        return fusionLightPalette();
    }
    case PaletteMode::Dark:
    {
        return fusionDarkPalette();
    }
    }
}

QStyle* StylesProvider::style(ThemeStyle style) const
{
    switch (style)
    {
    case ThemeStyle::System:
    {
        return QStyleFactory::create(SystemContext::theme.styleName);
    }
    case ThemeStyle::Fusion:
    {
        return QStyleFactory::create("Fusion");
    }
    }
}

QColor StylesProvider::iconColor(IconColor color) const
{
    switch (color)
    {
    case IconColor::System:
    {
        return SystemContext::theme.palette.color(QPalette::WindowText);
    }
    case IconColor::White:
    {
        return QColor(220,220,220);
    }
    case IconColor::Black:
    {
        return QColor(32,32,32);
    }
    }
}

QPalette StylesProvider::fusionLightPalette() const
{
    QPalette palette;

    // main surfaces
    palette.setColor(QPalette::Window,
                     QColor(212, 208, 200));   // Win2000 style

    palette.setColor(QPalette::Base,
                     QColor(236, 233, 216));

    palette.setColor(QPalette::AlternateBase,
                     QColor(224, 221, 213));

    // text
    palette.setColor(QPalette::WindowText,
                     QColor(32, 32, 32));

    palette.setColor(QPalette::Text,
                     QColor(32, 32, 32));

    palette.setColor(QPalette::ButtonText,
                     QColor(32, 32, 32));

    // buttons
    palette.setColor(QPalette::Button,
                     QColor(212, 208, 200));

    // selection
    palette.setColor(QPalette::Highlight,
                     QColor(10, 36, 106));

    palette.setColor(QPalette::HighlightedText,
                     QColor(240, 240, 240));

    // links
    palette.setColor(QPalette::Link,
                     QColor(0, 0, 180));

    // tooltips
    palette.setColor(QPalette::ToolTipBase,
                     QColor(255, 255, 225));

    palette.setColor(QPalette::ToolTipText,
                     QColor(32, 32, 32));

    // placeholders
    palette.setColor(QPalette::PlaceholderText,
                     QColor(110, 110, 110));

    // disabled
    palette.setColor(QPalette::Disabled,
                     QPalette::Text,
                     QColor(128, 128, 128));

    palette.setColor(QPalette::Disabled,
                     QPalette::ButtonText,
                     QColor(128, 128, 128));

    return palette;
}

QPalette StylesProvider::fusionDarkPalette() const
{
    QPalette palette;

    // main surfaces
    palette.setColor(QPalette::Window,
                     QColor(45, 48, 56));

    palette.setColor(QPalette::Base,
                     QColor(35, 38, 46));

    palette.setColor(QPalette::AlternateBase,
                     QColor(55, 58, 66));

    // text
    palette.setColor(QPalette::WindowText,
                     QColor(220, 220, 220));

    palette.setColor(QPalette::Text,
                     QColor(220, 220, 220));

    palette.setColor(QPalette::ButtonText,
                     QColor(220, 220, 220));

    // buttons
    palette.setColor(QPalette::Button,
                     QColor(55, 58, 66));

    // selection
    palette.setColor(QPalette::Highlight,
                     QColor(70, 95, 140));

    palette.setColor(QPalette::HighlightedText,
                     QColor(240, 240, 240));

    // links
    palette.setColor(QPalette::Link,
                     QColor(110, 150, 220));

    // tooltips
    palette.setColor(QPalette::ToolTipBase,
                     QColor(60, 60, 70));

    palette.setColor(QPalette::ToolTipText,
                     QColor(220, 220, 220));

    // placeholders
    palette.setColor(QPalette::PlaceholderText,
                     QColor(140, 140, 140));

    // disabled
    palette.setColor(QPalette::Disabled,
                     QPalette::Text,
                     QColor(120, 120, 120));

    palette.setColor(QPalette::Disabled,
                     QPalette::ButtonText,
                     QColor(120, 120, 120));

    return palette;
}