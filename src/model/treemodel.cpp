#include "model/treemodel.h"

TreeModel::TreeModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    m_root = std::make_unique<Node>(NodeType::Folder, "ROOT");

    auto trash = std::make_unique<Node>(NodeType::Trash); // setting title in data()

    m_trash = trash.get();

    trash->parent = m_root.get();

    m_root->children.push_back(std::move(trash));
}

Node *TreeModel::nodeFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return m_root.get();
    }

    return static_cast<Node*>(index.internalPointer());
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    Node *node = nodeFromIndex(parent);

    return node->children.size();
}

int TreeModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    auto *node = static_cast<Node*>(index.internalPointer());

    // show value and pass it to editor
    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        if (node->type == NodeType::Trash)
        {
            return tr("Trash");
        }

        return node->title;
    }

    return {};
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    Node *parentNode = nodeFromIndex(parent);

    if (row < 0 || row >= parentNode->children.size())
    {
        return {};
    }

    Node *child = parentNode->children[row].get();

    return createIndex(row, column, child);
}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return {};
    }

    auto *node = static_cast<Node*>(index.internalPointer());

    Node *parentNode = node->parent;

    if (!parentNode)
    {
        return {};
    }

    if (parentNode == m_root.get())
    {
        return {};
    }

    return createIndex(parentNode->row(), 0, parentNode);
}

Node* TreeModel::root() const
{
    return m_root.get();
}

Node* TreeModel::addFolder(const QModelIndex& parentIndex, const QString& title)
{
    Node* selectedNode = nodeFromIndex(parentIndex);

    Node* parentNode = m_insertionParent(selectedNode);

    QModelIndex insertIndex;

    if (parentNode != m_root.get())
    {
        insertIndex = createIndex(
            parentNode->row(),
            0,
            parentNode);
    }

    int row = static_cast<int>(parentNode->children.size());

    beginInsertRows(insertIndex, row, row);

    Node* node = parentNode->addFolder(title);

    endInsertRows();

    emit modified();

    return node;
}

Node* TreeModel::addEntry(const QModelIndex& parentIndex, const QString& title)
{
    Node* selectedNode = nodeFromIndex(parentIndex);

    Node* parentNode = m_insertionParent(selectedNode);

    QModelIndex insertIndex;

    if (parentNode != m_root.get())
    {
        insertIndex = createIndex(
            parentNode->row(),
            0,
            parentNode);
    }

    int row = static_cast<int>(parentNode->children.size());

    beginInsertRows(insertIndex, row, row);

    Node* node = parentNode->addEntry(title);

    endInsertRows();

    emit modified();

    return node;
}

Node* TreeModel::m_insertionParent(Node* node) const
{
    if (!node)
    {
        return m_root.get();
    }

    if (node->canHaveChildren())
    {
        return node;
    }

    return node->parent;
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled
           | Qt::ItemIsSelectable
           | Qt::ItemIsEditable;
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    Node* node = nodeFromIndex(index);

    if (!node)
    {
        return false;
    }

    if (node->type == NodeType::Trash)
    {
        return false;
    }

    if (role == Qt::EditRole)
    {
        QString newTitle = value.toString();

        if (newTitle.trimmed().isEmpty())
        {
            emit dataChanged(index, index);

            return false;
        }

        node->title = newTitle;
    }
    else if (role == ContentRole)
    {
        node->content = value.toString();
    }
    else
    {
        return false;
    }

    emit dataChanged(index, index);

    emit modified();

    return true;
}

bool TreeModel::renameNode(const QModelIndex& index, const QString& title)
{
    return setData(index, title, Qt::EditRole);
}

void TreeModel::deleteNodeOrMoveToTrash(Node* node)
{
    if (!node)
    {
        return;
    }

    if (node == m_root.get())
    {
        return;
    }

    if (isTrashNode(node))
    {
        return;
    }

    if (isInTrash(node))
    {
        deleteNode(node);
    }
    else
    {
        moveToTrash(node);
    }
}

void TreeModel::setRoot(std::unique_ptr<Node> root)
{
    beginResetModel();

    m_root = std::move(root);

    if (m_root)
    {
        m_fixParents(m_root.get());
    }

    m_trash = nullptr;

    if (m_root)
    {
        for (auto& child : m_root->children)
        {
            if (child->isTrash())
            {
                m_trash = child.get();

                break;
            }
        }
    }

    endResetModel();
}

void TreeModel::m_fixParents(Node* node)
{
    if (!node)
    {
        return;
    }

    for (auto& child : node->children)
    {
        child->parent = node;

        m_fixParents(child.get());
    }
}

// hard delete
void TreeModel::deleteNode(Node* node)
{
    if (!node)
    {
        return;
    }

    if (!isInTrash(node))
    {
        return;
    }

    Node* parent = node->parent;

    if (!parent)
    {
        return;
    }

    auto& siblings = parent->children;

    auto it = std::find_if(
        siblings.begin(),
        siblings.end(),
        [&](const auto& ptr)
        {
            return ptr.get() == node;
        });

    if (it == siblings.end())
    {
        return;
    }

    int row = std::distance(
        siblings.begin(),
        it);

    beginRemoveRows(
        indexFromNode(parent),
        row,
        row);

    siblings.erase(it);

    endRemoveRows();

    emit modified();
}

// soft delete
void TreeModel::moveToTrash(Node* node)
{
    if (!node)
    {
        return;
    }

    Node* oldParent = node->parent;

    if (!oldParent)
    {
        return;
    }

    int row = node->row();

    // detach from old parent
    auto& siblings = oldParent->children;

    auto it = std::find_if(
        siblings.begin(),
        siblings.end(),
        [&](const auto& ptr)
        {
            return ptr.get() == node;
        });

    if (it == siblings.end())
    {
        return;
    }

    QModelIndex oldParentIndex = indexFromNode(oldParent);

    beginRemoveRows(oldParentIndex, row, row);

    std::unique_ptr<Node> moved = std::move(*it);

    siblings.erase(it);

    endRemoveRows();

    Node* trash = m_trash;

    int trashRow = trash->children.size();

    QModelIndex trashIndex = indexFromNode(trash);

    beginInsertRows(trashIndex, trashRow, trashRow);

    // fix parent pointer
    moved->parent = trash;

    trash->children.push_back(std::move(moved));

    endInsertRows();

    emit modified();
}

// hard delete
void TreeModel::deleteNode(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return;
    }

    Node *node = nodeFromIndex(index);

    if (!node)
    {
        return;
    }

    // hard delete ONLY if node is in Trash subtree
    if (!isInTrash(node))
    {
        return;
    }

    Node *parent = node->parent;

    if (!parent)
    {
        return;
    }

    auto &siblings = parent->children;

    auto it = std::find_if(
        siblings.begin(),
        siblings.end(),
        [&](const std::unique_ptr<Node> &ptr)
        {
            return ptr.get() == node;
        });

    if (it == siblings.end())
    {
        return;
    }

    int row = std::distance(siblings.begin(), it);

    beginRemoveRows(indexFromNode(parent), row, row);

    siblings.erase(it);

    endRemoveRows();

    emit modified();
}

QModelIndex TreeModel::indexFromNode(Node* node) const
{
    if (!node)
    {
        return QModelIndex();
    }

    if (node == m_root.get())
    {
        return QModelIndex();
    }

    Node* parent = node->parent;

    return createIndex(
        node->row(),
        0,
        node);
}

bool TreeModel::setContent(const QModelIndex& index, const QString& content)
{
    Node* node = nodeFromIndex(index);

    if (!node)
    {
        return false;
    }

    if (!node->isEntry())
    {
        return false;
    }

    node->content = content;

    emit modified();

    return true;
}

void TreeModel::copyNode(const QModelIndex& index)
{
    Node* node = nodeFromIndex(index);

    if (!node)
    {
        return;
    }

    m_clipboard = node->clone();

    m_cutNode = nullptr;

    m_clipboardMode = ClipboardMode::Copy;
}

void TreeModel::pasteNode(const QModelIndex& targetIndex)
{
    if (m_clipboardMode == ClipboardMode::Copy)
    {
        pasteCopy(targetIndex);

        return;
    }

    if (m_clipboardMode == ClipboardMode::Cut)
    {
        pasteCut(targetIndex);

        return;
    }
}

void TreeModel::pasteCut(const QModelIndex& targetIndex)
{
    if (!m_cutNode)
    {
        return;
    }

    Node* source = m_cutNode;

    Node* target = nullptr;

    if (targetIndex.isValid())
    {
        target = nodeFromIndex(targetIndex);
    }

    Node* newParent = nullptr;

    if (!target)
    {
        newParent = m_root.get();
    }
    else if (target->isFolder())
    {
        newParent = target;
    }
    else
    {
        newParent = target->parent;
    }

    if (!newParent)
    {
        return;
    }

    if (newParent == source)
    {
        return;
    }

    if (isDescendant(newParent, source))
    {
        return;
    }

    Node* oldParent = source->parent;

    if (!oldParent)
    {
        return;
    }

    if (newParent == oldParent)
    {
        return;
    }

    // save state before moving
    int sourceRow = source->row();

    int destinationRow = static_cast<int>(newParent->children.size());

    QModelIndex sourceParentIndex = indexFromNode(oldParent);

    QModelIndex destinationParentIndex = indexFromNode(newParent);

    auto& siblings = oldParent->children;

    auto it = std::find_if(
        siblings.begin(),
        siblings.end(),
        [&](const auto& ptr)
        {
            return ptr.get() == source;
        });

    if (it == siblings.end())
    {
        return;
    }

    beginMoveRows(
        sourceParentIndex,
        sourceRow,
        sourceRow,
        destinationParentIndex,
        destinationRow);

    std::unique_ptr<Node> moved = std::move(*it);

    siblings.erase(it);

    moved->parent = newParent;

    newParent->children.push_back(std::move(moved));

    endMoveRows();

    m_cutNode = nullptr;

    emit modified();
}

bool TreeModel::isDescendant(Node* node, Node* ancestor) const
{
    while (node)
    {
        if (node == ancestor)
        {
            return true;
        }

        node = node->parent;
    }

    return false;
}

void TreeModel::pasteCopy(const QModelIndex& targetIndex)
{
    if (!m_clipboard)
    {
        return;
    }

    Node* target = nullptr;

    if (targetIndex.isValid())
    {
        target = nodeFromIndex(targetIndex);
    }

    Node* parent = nullptr;

    if (!target)
    {
        parent = m_root.get();
    }
    else if (target->isFolder())
    {
        parent = target;
    }
    else
    {
        parent = target->parent;
    }

    if (!parent)
    {
        return;
    }

    int row = parent->children.size();

    beginInsertRows(
        indexFromNode(parent),
        row,
        row);

    auto copy = m_clipboard->clone();

    copy->parent = parent;

    parent->children.push_back(std::move(copy));

    endInsertRows();

    emit modified();
}

void TreeModel::clear()
{
    auto root = std::make_unique<Node>(NodeType::Folder, "ROOT");

    auto trash = std::make_unique<Node>(NodeType::Trash, "Trash");

    trash->parent = root.get();

    root->children.push_back(std::move(trash));

    setRoot(std::move(root));

    emit modified();
}

void TreeModel::cutNode(const QModelIndex& index)
{
    Node* node = nodeFromIndex(index);

    if (!node)
    {
        return;
    }

    m_cutNode = node;

    m_clipboard.reset();

    m_clipboardMode = ClipboardMode::Cut;
}

bool TreeModel::isInTrash(Node* node) const
{
    while (node)
    {
        if (node == m_trash)
        {
            return true;
        }

        node = node->parent;
    }

    return false;
}

bool TreeModel::isTrashNode(Node* node) const
{
    return node == m_trash;
}