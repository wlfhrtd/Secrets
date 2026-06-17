#include <QFile>
#include <openssl/rand.h>

#include "vaultservice.h"
#include "model/treemodel.h"
#include "infrastructure/xmlstorage.h"


VaultService::VaultService(TreeModel* model)
    : m_model(model)
{
}

bool VaultService::loadFromFile(const QString& path, const QString& password)
{
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray fileData = file.readAll();

    VaultHeader header;

    QByteArray encrypted;

    if (!m_vaultFormat.unpack(
            fileData,
            header,
            encrypted))
    {
        return false;
    }

    QByteArray key = m_cryptoService.deriveKey(
        password,
        header);

    QByteArray xml = m_cryptoService.decrypt(
        encrypted,
        key);

    if (xml.isEmpty())
    {
        return false;
    }

    auto root = m_xmlStorage.load(xml);

    m_model->setRoot(std::move(root));

    return true;
}

bool VaultService::saveToFile(const QString& path, const QString& password)
{
    QByteArray xml = m_xmlStorage.save(m_model->root());

    VaultHeader header;

    header.salt = m_cryptoService.generateSalt();

    QByteArray key = m_cryptoService.deriveKey(
        password,
        header);

    QByteArray encrypted = m_cryptoService.encrypt(
        xml,
        key);

    QByteArray fileData = m_vaultFormat.pack(
        header,
        encrypted);

    QFile file(path);

    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    file.write(fileData);

    return true;
}

bool VaultService::exportXmlFile(const QString& path)
{
    if (!m_model)
    {
        return false;
    }

    Node* root = m_model->root();

    if (!root)
    {
        return false;
    }

    QByteArray xml = m_xmlStorage.save(root);

    QFile file(path);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        return false;
    }

    if (file.write(xml) != xml.size())
    {
        return false;
    }

    return file.flush();
}