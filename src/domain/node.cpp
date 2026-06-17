#include "domain/node.h"


Node::Node(NodeType type)
    : type(type)
{
}

Node::Node(NodeType type, QString title)
    : type(type)
    , title(std::move(title))
{
}


bool Node::isFolder() const
{
    return type == NodeType::Folder;
}

bool Node::isEntry() const
{
    return type == NodeType::Entry;
}

bool Node::isTrash() const
{
    return type == NodeType::Trash;
}

int Node::row() const
{
    if (!parent)
    {
        return 0;
    }

    for (int i = 0; i < parent->children.size(); ++i)
    {
        if (parent->children[i].get() == this)
        {
            return i;
        }
    }

    return 0;
}

Node* Node::addFolder(const QString& title)
{
    auto node = std::make_unique<Node>(NodeType::Folder, title);

    node->parent = this;

    Node* ptr = node.get();

    children.push_back(std::move(node));

    return ptr;
}

Node* Node::addEntry(const QString& title)
{
    auto node = std::make_unique<Node>(NodeType::Entry, title);

    node->parent = this;

    Node* ptr = node.get();

    children.push_back(std::move(node));

    return ptr;
}

void Node::removeChild(Node* child)
{
    auto it = std::find_if(
        children.begin(),
        children.end(),
        [child](const auto& ptr)
        {
            return ptr.get() == child;
        });

    if (it != children.end())
    {
        children.erase(it);
    }
}

bool Node::canHaveChildren() const
{
    return type != NodeType::Entry;
}

std::unique_ptr<Node> Node::clone() const
{
    auto copy = std::make_unique<Node>(
        type,
        title);

    copy->content = content;

    for (const auto& child : children)
    {
        auto childCopy = child->clone();

        childCopy->parent = copy.get();

        copy->children.push_back(
            std::move(childCopy));
    }

    return copy;
}