#ifndef VAULTHEADER_H
#define VAULTHEADER_H

#pragma once

#include <QByteArray>


struct VaultHeader
{
    static constexpr char MAGIC[] = "SVDB";
    static constexpr int MAGIC_SIZE = sizeof(MAGIC) - 1;

    quint8 version = 1;

    QByteArray salt;

    // Argon2 KDF params
    quint32 iterations = 3;
    quint32 memory = 65536;
    quint32 parallelism = 1;
};

#endif // VAULTHEADER_H
