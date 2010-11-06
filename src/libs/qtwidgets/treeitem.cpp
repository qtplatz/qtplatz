//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "treeitem.h"
#include <QStringList>
#include "treeitem.h"
#include <algorithm>

using namespace qtwidgets;

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
