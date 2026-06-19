#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#pragma once

#include "ui/icons/iconcache.h"
#include "ui/icons/iconid.h"
#include <QColor>
#include <QIcon>


class IconProvider
{
public:
    IconProvider();

    QIcon icon(IconId id, const QColor& color);

    void clearCache();

private:
    static QString iconPath(IconId id);

    QPixmap renderSvgColored(
        const QString& path,
        const QSize& size,
        const QColor& color) const;

    QIcon recolorIcon(const QString& path, const QColor& color) const;

    IconCache m_cache;
};

#endif // ICONPROVIDER_H