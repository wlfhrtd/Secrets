#include "thememanager.h"

#include <QStyleFactory>
#include <qapplication.h>
#include <shared/systemcontext.h>


void ThemeManager::apply(ThemeMode mode)
{
    qApp->setStyleSheet("");

    if (mode == ThemeMode::Light)
    {
        applyLightTheme();

        return;
    }

    if (mode == ThemeMode::Dark)
    {
        applyDarkTheme();

        return;
    }

    applySystemTheme();
}

void ThemeManager::applySystemTheme()
{
    qApp->setStyle(QStyleFactory::create(SystemContext::theme.styleName));
    qApp->setPalette(SystemContext::theme.palette);

    m_uiIconColor = SystemContext::theme.palette.color(QPalette::WindowText);
    m_systemIconColor = SystemContext::theme.palette.color(QPalette::WindowText);
}

void ThemeManager::applyLightTheme()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(createFusionLightPalette());

    m_uiIconColor = QColor(32,32,32); // black

#ifdef Q_OS_WIN
    // On Windows Fusion Light produces light title bar
    // so the window icon should be dark as well
    m_systemIconColor = QColor(32,32,32); // black
#else
    // On Linux the title bar is usually controlled by the
    // window manager (KDE/GNOME/etc.), not by Fusion.
    m_systemIconColor = SystemContext::theme.palette.color(QPalette::WindowText);
#endif
}

void ThemeManager::applyDarkTheme()
{
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setPalette(createFusionDarkPalette());

    m_uiIconColor = QColor(220,220,220); // white

#ifdef Q_OS_WIN
    // On Windows Fusion Dark produces dark title bar
    m_systemIconColor = QColor(220,220,220); // white
#else
    // On Linux keep following the system/window manager
    m_systemIconColor = SystemContext::theme.palette.color(QPalette::WindowText);
#endif
}

QColor ThemeManager::uiIconColor() const
{
    return m_uiIconColor;
}

QColor ThemeManager::systemIconColor() const
{
    return m_systemIconColor;
}

QPalette ThemeManager::createFusionLightPalette() const
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

QPalette ThemeManager::createFusionDarkPalette() const
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