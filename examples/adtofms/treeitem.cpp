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

/*
    treeitem.cpp

    A container for items of data supplied by the simple tree model.
*/

#include <QStringList>
#include "treeitem.h"
#include <algorithm>

TreeItem::TreeItem(const std::vector<QVariant> &data, TreeItem *parent) : parentItem_(parent)
                                                                        , itemData_(data)
{
}

TreeItem::TreeItem( const TreeItem& t ) : parentItem_(t.parentItem_), itemData_(t.itemData_)
{
}

TreeItem::~TreeItem()
{
}

TreeItemPtr
TreeItem::child(int number)
{
    if ( childItems_.size() <= unsigned(number) )
        return TreeItemPtr();
    return childItems_[number];
}

int
TreeItem::childCount() const
{
    return childItems_.size();
}

int
TreeItem::childNumber() const
{
    if ( parentItem_ ) {
        for ( vector_type::iterator it = parentItem_->begin(); it != parentItem_->end(); ++it ) {
            if ( it->get() == this )
                return std::distance( parentItem_->begin(), it );
        }
    }
    return 0;
}

int
TreeItem::columnCount() const
{
    return itemData_.size();
}

QVariant
TreeItem::data(int column) const
{
    if ( itemData_.size() <= unsigned(column) )
        return QVariant();
    return itemData_[column];
}

bool
TreeItem::insertChildren( int position, int count, int columns )
{
    if (position < 0 || unsigned(position) > childItems_.size())
        return false;

    for (int row = 0; row < count; ++row) {
        std::vector<QVariant> data(columns);
        TreeItemPtr ptr( new TreeItem( data, this ) );
        childItems_.insert( childItems_.begin() + position, ptr );
    }

    return true;
}

bool
TreeItem::insertColumns(int position, int columns)
{
    if (position < 0 || unsigned(position) > itemData_.size())
        return false;

    for (int column = 0; column < columns; ++column)
        itemData_.insert( itemData_.begin() + position, QVariant());

    for ( vector_type::iterator it = childItems_.begin(); it != childItems_.end(); ++it )
        (*it)->insertColumns( position, columns );

    return true;
}

TreeItem *
TreeItem::parent()
{
    return parentItem_;
}

bool
TreeItem::removeChildren( int position, int count )
{
    if (position < 0 || unsigned(position + count) > childItems_.size())
        return false;
    childItems_.erase( childItems_.begin() + position, childItems_.begin() + position + count );
    return true;
}
//! [10]

bool TreeItem::removeColumns(int position, int columns)
{
    if (position < 0 || unsigned( position + columns ) > itemData_.size())
        return false;

    itemData_.erase( itemData_.begin() + position, itemData_.begin() + position + columns );

    for ( vector_type::iterator it = childItems_.begin(); it != childItems_.end(); ++it )
         (*it)->removeColumns( position, columns );

    return true;
}

//! [11]
bool TreeItem::setData(int column, const QVariant &value)
{
    if (column < 0 || unsigned( column ) >= itemData_.size())
        return false;

    itemData_[column] = value;
    return true;
}
//! [11]
