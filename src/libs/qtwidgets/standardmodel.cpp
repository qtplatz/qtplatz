// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "standardmodel.hpp"
#include "treeitem.hpp"
#include <adcontrols/centroidmethod.hpp>

using namespace qtwidgets;

StandardModel::StandardModel(QObject *parent) :
                 QAbstractItemModel(parent)
{
    std::vector< QVariant > rootData;
    rootData.push_back( "Centroid" );
    for ( int i = 0; i < 2; ++i )
        rootData.push_back( "" );
    rootItem_.reset( new TreeItem(rootData) );
}

StandardModel::~StandardModel()
{
}

////////////////////////////////////////////////////////
int
StandardModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem_->columnCount();
}

QVariant
StandardModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeItem * item = getItem(index);
    return item->data(index.column());
}

Qt::ItemFlags
StandardModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

TreeItem *
StandardModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem * item = static_cast<TreeItem *>(index.internalPointer());
        if (item) return item;
    }
    return rootItem_.get();
}

QVariant 
StandardModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem_->data(section);

    return QVariant();
}

QModelIndex 
StandardModel::index(int row, int column, const QModelIndex &parent) const
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
StandardModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    bool success;

    beginInsertColumns(parent, position, position + columns - 1);
    success = rootItem_->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool
StandardModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem * parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem_->columnCount());
    endInsertRows();

    return success;
}

QModelIndex
StandardModel::parent(const QModelIndex &index) const
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
StandardModel::removeColumns(int position, int columns, const QModelIndex &parent)
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
StandardModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem * parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

int
StandardModel::rowCount(const QModelIndex &parent) const
{
    TreeItem * parentItem = getItem(parent);
    return parentItem->childCount();
}

bool
StandardModel::setData(const QModelIndex &index, const QVariant &value, int role)
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
StandardModel::setHeaderData(int section, Qt::Orientation orientation,
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
StandardModel::findParent( const QString& addrString ) const
{
    TreeItem::vector_type::const_iterator begIt = rootItem_->begin();
    for ( TreeItem::vector_type::const_iterator it = begIt; it != rootItem_->end(); ++it ) {
        QVariant v = (*it)->data(0);
        if ( addrString == v )
            return std::distance( begIt, it );
    }
    return (-1);
}
