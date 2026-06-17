#ifndef VAULTSESSION_H
#define VAULTSESSION_H

#pragma once

#include <QString>

class VaultSession
{
public:
    void clear();

    bool isDirty() const;
    void setDirty(bool value);

    const QString& path() const;
    void setPath(const QString& path);

    const QString& password() const;
    void setPassword(const QString& password);

private:
    QString m_path;
    QString m_password;

    bool m_dirty = false;
};

#endif // VAULTSESSION_H
