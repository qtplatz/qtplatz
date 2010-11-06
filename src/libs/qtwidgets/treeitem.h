// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

#include <QVariant>
#include <vector>
#include <boost/shared_ptr.hpp>

namespace qtwidgets {

    class TreeItem;
    typedef boost::shared_ptr<TreeItem> TreeItemPtr;
    
    class TreeItem {
    public:
        TreeItem(const std::vector< QVariant >&data, TreeItem * parent = 0);
        TreeItem( const TreeItem& );
        ~TreeItem();
        
        typedef std::vector< boost::shared_ptr<TreeItem> > vector_type;
        
        inline vector_type::iterator begin() { return childItems_.begin(); }
        inline vector_type::iterator end() { return childItems_.end(); }
        inline vector_type::const_iterator begin() const { return childItems_.begin(); }
        inline vector_type::const_iterator end() const { return childItems_.end(); }
        
        TreeItemPtr child(int number);
        int childCount() const;
        int columnCount() const;
        QVariant data(int column) const;
        bool insertChildren(int position, int count, int columns);
        bool insertColumns(int position, int columns);
        TreeItem * parent();
        bool removeChildren(int position, int count);
        bool removeColumns(int position, int columns);
        int childNumber() const;
        bool setData(int column, const QVariant &value);
        
    private:
        //QList<TreeItem*> childItems_;
        std::vector< QVariant > itemData_;
        vector_type childItems_;
        TreeItem * parentItem_;
    };
    
}
