#ifndef TREEFILTERPROXYMODEL_H
#define TREEFILTERPROXYMODEL_H

#pragma once

#include <QSortFilterProxyModel>
#include <domain/node.h>


class TreeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit TreeFilterProxyModel(QObject *parent = nullptr);

    void setShowTrash(bool value);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    bool hasAcceptedChildren(const QModelIndex& parent) const;

    // filter children if parent matched
    bool hasMatchingParent(Node* node, const QString& filter) const;

    // sort
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    bool m_showTrash = false;
};

#endif // TREEFILTERPROXYMODEL_H
