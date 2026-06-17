#pragma once

#include <QAbstractItemModel>
#include "domain/node.h"


enum class ClipboardMode
{
    Copy,
    Cut
};

enum Roles
{
    TitleRole = Qt::UserRole + 1,
    ContentRole
};

class TreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TreeModel(QObject *parent = nullptr);
    ~TreeModel() override = default;

    // create index for child
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    // number of children
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // what to display: title
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Node* root() const;

    // CRUD
    Node* addFolder(const QModelIndex& parentIndex, const QString& title);
    Node* addEntry(const QModelIndex& parentIndex, const QString& title);

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    bool renameNode(const QModelIndex& index, const QString& title);

    void moveToTrash(Node* node);
    void deleteNodeOrMoveToTrash(Node* node);
    void deleteNode(const QModelIndex &index);
    void deleteNode(Node* node);

    Node* nodeFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromNode(Node *node) const;

    bool setContent(const QModelIndex& index,const QString& content);

    void copyNode(const QModelIndex& index);

    void pasteNode(const QModelIndex& targetIndex);

    void cutNode(const QModelIndex& index);

    void pasteCopy(const QModelIndex& targetIndex);

    void pasteCut(const QModelIndex& targetIndex);

    bool isDescendant(Node* node, Node* ancestor) const;

    bool isInTrash(Node* node) const;

    bool isTrashNode(Node* node) const;

    void setRoot(std::unique_ptr<Node> root);

    void clear(); // creates new ROOT with Trash

private:
    std::unique_ptr<Node> m_root;

    Node* m_trash = nullptr;

    // UX logic: if current node can't have children - parent can
    Node* m_insertionParent(Node* node) const;

    std::unique_ptr<Node> m_clipboard;

    Node* m_cutNode = nullptr;

    ClipboardMode m_clipboardMode = ClipboardMode::Copy;

    void m_fixParents(Node* node);

signals:
    void modified();
};