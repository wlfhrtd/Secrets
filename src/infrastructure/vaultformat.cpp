#include "vaultformat.h"

#include <QByteArray>
#include <QIODevice>

QByteArray VaultFormat::pack(const VaultHeader& header, const QByteArray& encrypted)
{
    QByteArray out;

    QDataStream stream(&out, QIODevice::WriteOnly);

    stream.writeRawData(VaultHeader::MAGIC, VaultHeader::MAGIC_SIZE);

    stream << header.version;

    stream << header.iterations;
    stream << header.memory;
    stream << header.parallelism;

    stream.writeRawData(header.salt.constData(), header.salt.size());

    stream.writeRawData(encrypted.constData(), encrypted.size());

    return out;
}

bool VaultFormat::unpack(const QByteArray& fileData, VaultHeader& header, QByteArray& encrypted)
{
    QDataStream stream(fileData);

    char magic[VaultHeader::MAGIC_SIZE];

    stream.readRawData(magic, VaultHeader::MAGIC_SIZE);

    if (memcmp(magic, VaultHeader::MAGIC, VaultHeader::MAGIC_SIZE) != 0)
    {
        return false;
    }

    stream >> header.version;

    stream >> header.iterations;
    stream >> header.memory;
    stream >> header.parallelism;

    header.salt.resize(16);

    stream.readRawData(header.salt.data(), 16);

    encrypted = fileData.mid(stream.device()->pos());

    return true;
}