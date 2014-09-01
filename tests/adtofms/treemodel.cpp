/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "treeitem.h"
#include "treemodel.h"

TreeModel::TreeModel( QObject *parent ) : QAbstractItemModel(parent)
{
    std::vector< QVariant > rootData;
    rootData.push_back( "Device Name" );
    rootData.push_back( "Description 1" );
    rootData.push_back( "Description 2" );

    rootItem_.reset( new TreeItem(rootData) );
}

TreeModel::~TreeModel()
{
}

int
TreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem_->columnCount();
}

QVariant
TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem * item = getItem(index);
    return item->data(index.column());
}

Qt::ItemFlags
TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

TreeItem *
TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem * item = static_cast<TreeItem *>(index.internalPointer());
        if (item) return item;
    }
    return rootItem_.get();
}

QVariant 
TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem_->data(section);

    return QVariant();
}

QModelIndex 
TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem * parentItem = getItem(parent);
    TreeItemPtr childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem.get());
    else
        return QModelIndex();
}

bool 
TreeModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem_->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool
TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem * parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem_->columnCount());
    endInsertRows();

    return success;
}

QModelIndex
TreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem * childItem = getItem(index);
    TreeItem * parentItem = childItem->parent();

    if ( parentItem == rootItem_.get() )
        return QModelIndex();

    return createIndex( parentItem->childNumber(), 0, parentItem );
}

bool
TreeModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginRemoveColumns(parent, position, position + columns - 1);
    success = rootItem_->removeColumns(position, columns);
    endRemoveColumns();

    if (rootItem_->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}

bool
TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem * parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int
TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem * parentItem = getItem(parent);
    return parentItem->childCount();
}

bool
TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem * item = getItem(index);
    bool result = item->setData(index.column(), value);

    if (result)
        emit dataChanged(index, index);

    return result;
}

bool
TreeModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    bool result = rootItem_->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

int
TreeModel::findParent( const QString& addrString ) const
{
    TreeItem::vector_type::const_iterator begIt = rootItem_->begin();
    for ( TreeItem::vector_type::const_iterator it = begIt; it != rootItem_->end(); ++it ) {
        QVariant v = (*it)->data(0);
        if ( addrString == v )
            return std::distance( begIt, it );
    }
    return (-1);
}