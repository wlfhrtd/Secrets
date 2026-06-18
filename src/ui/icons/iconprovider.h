#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#pragma once

#include "ui/icons/IconId.h"
#include <QColor>
#include <QIcon>


// struct NoteToolBarIcons
// {
//     QIcon copySelected;
//     QIcon reveal;
//     QIcon stopwatch;
//     QIcon hide;
//     QIcon edit;
//     QIcon save;
// };

class IconProvider
{
public:
    IconProvider();

    QIcon icon(IconId id, const QColor& color) const;

    // void setUiColor(const QColor& color);
    // void setSystemColor(const QColor& color);

    // QIcon ui(IconId id) const;
    // QIcon system(IconId id) const;

    // QIcon windowIcon() const;

    // const NoteToolBarIcons& noteToolBarIcons() const;

private:
    static QString iconPath(IconId id);

    // QColor m_uiColor;
    // QColor m_systemColor;

    QPixmap renderSvgColored(
        const QString& path,
        const QSize& size,
        const QColor& color) const;

    QIcon recolorIcon(const QString& path, const QColor& color) const;

    // NoteToolBarIcons m_noteIcons;

    // void rebuildCache();
};

#endif // ICONPROVIDER_H
