#ifndef ICONCACHE_H
#define ICONCACHE_H

#pragma once

#include "ui/icons/iconid.h"

#include <QHash>
#include <QIcon>


class IconCache
{
public:
    bool contains(IconId id) const;

    QIcon get(IconId id) const;

    void put(IconId id, const QIcon& icon);

    void clear();

private:
    QHash<IconId, QIcon> m_icons;
};

#endif // ICONCACHE_H