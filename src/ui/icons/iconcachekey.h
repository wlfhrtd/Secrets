#ifndef ICONCACHEKEY_H
#define ICONCACHEKEY_H

#pragma once

#include "ui/icons/iconid.h"

#include <QColor>


struct IconCacheKey
{
    IconId id;
    QColor color;

    bool operator==(const IconCacheKey& other) const
    {
        return id == other.id
               && color == other.color;
    }
};

inline size_t qHash(const IconCacheKey& key, size_t seed = 0)
{
    seed ^= ::qHash(
        static_cast<int>(key.id),
        seed);

    seed ^= ::qHash(
        key.color.rgba(),
        seed << 1);

    return seed;
}

#endif // ICONCACHEKEY_H