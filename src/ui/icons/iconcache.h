#ifndef ICONCACHE_H
#define ICONCACHE_H

#pragma once

#include "ui/icons/iconid.h"
#include "ui/icons/iconcachekey.h"

#include <QHash>
#include <QIcon>


class IconCache
{
public:
    bool contains(IconId id, const QColor& color) const;

    QIcon get(IconId id, const QColor& color) const;

    void put(IconId id, const QColor& color, const QIcon& icon);

    void clear();

private:
    QHash<IconCacheKey, QIcon> m_icons;
};

#endif // ICONCACHE_H