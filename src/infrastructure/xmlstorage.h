#ifndef XMLSTORAGE_H
#define XMLSTORAGE_H

#pragma once

#include <QIODevice>
#include <QXmlStreamWriter>
#include <domain/node.h>


/*
 * Data structure concerns only
 * Node <-> XML
 */
class XmlStorage
{
public:
    QByteArray save(Node* root);
    std::unique_ptr<Node> load(const QByteArray& xml);

private:
    void writeNode(QXmlStreamWriter& writer, Node* node);
    std::unique_ptr<Node> readNode(QXmlStreamReader& reader);

    std::unique_ptr<Node> readEntry(QXmlStreamReader& reader);
    std::unique_ptr<Node> readFolder(QXmlStreamReader& reader);
    std::unique_ptr<Node> readTrash(QXmlStreamReader& reader);
};
#endif // XMLSTORAGE_H