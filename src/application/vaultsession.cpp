#include "vaultsession.h"

void VaultSession::clear()
{
    m_path.clear();
    m_password.clear();

    m_dirty = false;
}

bool VaultSession::isDirty() const
{
    return m_dirty;
}

void VaultSession::setDirty(bool value)
{
    m_dirty = value;
}

const QString& VaultSession::path() const
{
    return m_path;
}

void VaultSession::setPath(const QString& path)
{
    m_path = path;
}

const QString& VaultSession::password() const
{
    return m_password;
}

void VaultSession::setPassword(const QString& password)
{
    m_password = password;
}