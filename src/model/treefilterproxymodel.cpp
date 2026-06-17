#include "model/treefilterproxymodel.h"
#include "domain/node.h"


TreeFilterProxyModel::TreeFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}


bool TreeFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    QModelIndex index = sourceModel()->index(
        sourceRow,
        0,
        sourceParent);

    auto* node = static_cast<Node*>(index.internalPointer());

    if (!node)
    {
        return false;
    }

    // Trash visibility
    if (node->type == NodeType::Trash && !m_showTrash)
    {
        return false;
    }

    QString filter = filterRegularExpression().pattern();

    if (filter.isEmpty())
    {
        return true;
    }

    // show node with matching parent
    if (hasMatchingParent(node, filter))
    {
        return true;
    }

    // show node if matched
    if (node->title.contains(filter, Qt::CaseInsensitive))
    {
        return true;
    }

    // show matched children
    return hasAcceptedChildren(index);
}

bool TreeFilterProxyModel::hasAcceptedChildren(const QModelIndex& parent) const
{
    int rows = sourceModel()->rowCount(parent);

    for (int i = 0; i < rows; ++i)
    {
        QModelIndex child = sourceModel()->index(
            i,
            0,
            parent);

        auto* node = static_cast<Node*>(child.internalPointer());

        if (!node)
        {
            continue;
        }

        QString filter = filterRegularExpression().pattern();

        if (filter.isEmpty())
        {
            return true;
        }

        if (node->title.contains(filter, Qt::CaseInsensitive))
        {
            return true;
        }

        if (hasAcceptedChildren(child))
        {
            return true;
        }
    }

    return false;
}

bool TreeFilterProxyModel::hasMatchingParent(Node* node, const QString& filter) const
{
    Node* parent = node->parent;

    while (parent)
    {
        if (parent->type == NodeType::Folder
            && parent->title.contains(filter, Qt::CaseInsensitive))
        {
            return true;
        }

        parent = parent->parent;
    }

    return false;
}

bool TreeFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    auto* lhs = static_cast<Node*>(left.internalPointer());

    auto* rhs = static_cast<Node*>(right.internalPointer());

    if (!lhs || !rhs)
    {
        return false;
    }

    // Trash always last
    if (lhs->type == NodeType::Trash)
    {
        return false;
    }

    if (rhs->type == NodeType::Trash)
    {
        return true;
    }

    bool lhsFolder = lhs->type == NodeType::Folder;

    bool rhsFolder = rhs->type == NodeType::Folder;

    // folders before entries
    if (lhsFolder != rhsFolder)
    {
        return lhsFolder;
    }

    return QString::compare(
               lhs->title,
               rhs->title,
               Qt::CaseInsensitive) < 0;
}

void TreeFilterProxyModel::setShowTrash(bool value)
{
    if (m_showTrash == value)
    {
        return;
    }

    beginFilterChange();

    m_showTrash = value;

    endFilterChange();
}