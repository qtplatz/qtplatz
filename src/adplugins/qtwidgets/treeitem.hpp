// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#pragma once

#include <QVariant>
#include <vector>
#include <memory>

namespace qtwidgets {

    class TreeItem;

    typedef std::shared_ptr<TreeItem> TreeItemPtr;
    
    class TreeItem {
    public:
        TreeItem(const std::vector< QVariant >&data, TreeItem * parent = 0);
        TreeItem( const TreeItem& );
        ~TreeItem();
        
        typedef std::vector< std::shared_ptr<TreeItem> > vector_type;
        
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
