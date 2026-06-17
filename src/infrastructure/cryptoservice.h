#ifndef CRYPTOSERVICE_H
#define CRYPTOSERVICE_H

#pragma once

#include "vaultheader.h"

#include <QByteArray>
#include <QString>


/*
 * Cryptography operations only
 * QByteArray -> QByteArray
 */
class CryptoService
{
public:
    QByteArray encrypt(const QByteArray& plaintext, const QByteArray& key);
    QByteArray decrypt(const QByteArray& data, const QByteArray& key);

    QByteArray deriveKey(const QString& password, const VaultHeader& vaultHeader);

    QByteArray generateSalt();
};

#endif // CRYPTOSERVICE_H
