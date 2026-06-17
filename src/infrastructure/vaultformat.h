#ifndef VAULTFORMAT_H
#define VAULTFORMAT_H

#pragma once

#include "vaultheader.h"
#include <QByteArray>


class VaultFormat
{
public:
    QByteArray pack(
        const VaultHeader& header,
        const QByteArray& encrypted);

    bool unpack(
        const QByteArray& fileData,
        VaultHeader& header,
        QByteArray& encrypted);
};

#endif // VAULTFORMAT_H
