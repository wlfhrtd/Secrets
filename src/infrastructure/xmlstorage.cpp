#include "xmlstorage.h"


QByteArray XmlStorage::save(Node* root)
{
    QByteArray buffer;
    QXmlStreamWriter writer(&buffer);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();

    writer.writeStartElement("secrets");
    writer.writeAttribute("version", "1");

    for (const auto& child : root->children)
    {
        writeNode(writer, child.get());
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    return buffer;
}

void XmlStorage::writeNode(QXmlStreamWriter& writer, Node* node)
{
    switch (node->type)
    {
    case NodeType::Folder:
    {
        writer.writeStartElement("folder");

        writer.writeTextElement("title", node->title);

        for (const auto& child : node->children)
        {
            writeNode(writer, child.get());
        }

        writer.writeEndElement();

        break;
    }

    case NodeType::Entry:
    {
        writer.writeStartElement("entry");

        writer.writeTextElement("title", node->title);

        writer.writeTextElement("content", node->content);

        writer.writeEndElement();

        break;
    }

    case NodeType::Trash:
    {
        writer.writeStartElement("trash");

        for (const auto& child : node->children)
        {
            writeNode(writer, child.get());
        }

        writer.writeEndElement();

        break;
    }
    }
}

std::unique_ptr<Node> XmlStorage::readNode(QXmlStreamReader& reader)
{
    const auto name = reader.name();

    if (name == "folder")
    {
        return readFolder(reader);
    }

    if (name == "entry")
    {
        return readEntry(reader);
    }

    if (name == "trash")
    {
        return readTrash(reader);
    }

    return nullptr;
}

std::unique_ptr<Node> XmlStorage::load(const QByteArray& xml)
{
    QXmlStreamReader reader(xml);

    auto root = std::make_unique<Node>(NodeType::Folder, "ROOT");

    while (!reader.atEnd())
    {
        reader.readNext();

        if (!reader.isStartElement())
        {
            continue;
        }

        if (reader.name() == "secrets")
        {
            const QString version = reader.attributes()
                                        .value("version")
                                        .toString();

            if (version != "1")
            {
                return nullptr;
            }

            continue;
        }

        auto child = readNode(reader);

        if (!child)
        {
            return nullptr;
        }

        child->parent = root.get();

        root->children.push_back(std::move(child));
    }

    if (reader.hasError())
    {
        return nullptr;
    }

    return root;
}

std::unique_ptr<Node> XmlStorage::readEntry(QXmlStreamReader& reader)
{
    auto node = std::make_unique<Node>();

    node->type = NodeType::Entry;

    while (!reader.atEnd() && !reader.hasError())
    {
        reader.readNext();

        if (reader.isEndElement() && reader.name() == "entry")
        {
            break;
        }

        if (!reader.isStartElement())
        {
            continue;
        }

        if (reader.name() == "title")
        {
            node->title = reader.readElementText();
        }
        else if (reader.name() == "content")
        {
            node->content = reader.readElementText();
        }
    }

    return reader.hasError() ? nullptr : std::move(node);
}

std::unique_ptr<Node> XmlStorage::readFolder(QXmlStreamReader& reader)
{
    auto node = std::make_unique<Node>();

    node->type = NodeType::Folder;

    while (!reader.atEnd() && !reader.hasError())
    {
        reader.readNext();

        if (reader.isEndElement() && reader.name() == "folder")
        {
            break;
        }

        if (!reader.isStartElement())
        {
            continue;
        }

        if (reader.name() == "title")
        {
            node->title = reader.readElementText();
        }
        else if (reader.name() == "folder")
        {
            auto child = readFolder(reader);

            if (!child)
            {
                return nullptr;
            }

            child->parent = node.get();

            node->children.push_back(std::move(child));
        }
        else if (reader.name() == "entry")
        {
            auto child = readEntry(reader);

            if (!child)
            {
                return nullptr;
            }

            child->parent = node.get();

            node->children.push_back(std::move(child));
        }
    }

    return reader.hasError() ? nullptr : std::move(node);
}

std::unique_ptr<Node> XmlStorage::readTrash(QXmlStreamReader& reader)
{
    auto node = std::make_unique<Node>(NodeType::Trash, "Trash");

    while (!reader.atEnd() && !reader.hasError())
    {
        reader.readNext();

        if (reader.isEndElement() && reader.name() == "trash")
        {
            break;
        }

        if (!reader.isStartElement())
        {
            continue;
        }

        if (reader.name() == "title")
        {
            node->title = reader.readElementText();
        }
        else if (reader.name() == "folder")
        {
            auto child = readFolder(reader);

            if (!child)
            {
                return nullptr;
            }

            child->parent = node.get();

            node->children.push_back(std::move(child));
        }
        else if (reader.name() == "entry")
        {
            auto child = readEntry(reader);

            if (!child)
            {
                return nullptr;
            }

            child->parent = node.get();

            node->children.push_back(std::move(child));
        }
    }

    return reader.hasError() ? nullptr : std::move(node);
}