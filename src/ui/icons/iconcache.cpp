#include "iconcache.h"

bool IconCache::contains(IconId id, const QColor& color) const
{
    return m_icons.contains(IconCacheKey{id, color});
}

QIcon IconCache::get(IconId id, const QColor& color) const
{
    return m_icons.value(IconCacheKey{id, color});
}

void IconCache::put(IconId id, const QColor& color, const QIcon& icon)
{
    m_icons.insert(IconCacheKey{id, color}, icon);
}

void IconCache::clear()
{
    m_icons.clear();
}