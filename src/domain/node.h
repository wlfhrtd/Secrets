#ifndef NODE_H
#define NODE_H

#pragma once

#include <QString>
#include <vector>

enum class NodeType
{
    Folder,
    Entry,
    Trash,
};

struct Node
{
    Node() = default;
    Node(NodeType type);
    Node(NodeType type, QString title);

    NodeType type = NodeType::Entry;

    QString title;
    QString content; // only for Entry

    Node *parent = nullptr;

    std::vector<std::unique_ptr<Node>> children;

    bool isFolder() const;
    bool isEntry() const;
    bool isTrash() const;

    Node* addFolder(const QString& title);
    Node* addEntry(const QString& title);

    void removeChild(Node* child);

    bool canHaveChildren() const; // if not Entry

    std::unique_ptr<Node> clone() const;

    int row() const;
};
#endif // NODE_H