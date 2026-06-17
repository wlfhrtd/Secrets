#ifndef VAULTSERVICE_H
#define VAULTSERVICE_H

#pragma once

#include <QString>

#include <model/treemodel.h>
#include <infrastructure/cryptoservice.h>
#include <infrastructure/vaultformat.h>
#include <infrastructure/xmlstorage.h>


/*
 * Orchestration + File
 */
class VaultService
{
public:
    explicit VaultService(TreeModel* model);

    bool loadFromFile(const QString& path, const QString& password);

    bool saveToFile(const QString& path, const QString& password);

    bool exportXmlFile(const QString& path);

private:
    TreeModel* m_model = nullptr;
    CryptoService m_cryptoService;
    XmlStorage m_xmlStorage;
    VaultFormat m_vaultFormat;
};

#endif // VAULTSERVICE_H