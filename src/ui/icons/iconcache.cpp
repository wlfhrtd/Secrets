#include "iconcache.h"


bool IconCache::contains(IconId id) const
{
    return m_icons.contains(id);
}

QIcon IconCache::get(IconId id) const
{
    return m_icons.value(id);
}

void IconCache::put(IconId id, const QIcon& icon)
{
    m_icons.insert(id, icon);
}

void IconCache::clear()
{
    m_icons.clear();
}