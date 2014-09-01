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

#ifndef TREEITEM_H
#define TREEITEM_H

#include <QVariant>
#include <vector>
#include <boost/shared_ptr.hpp>

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
//! [0]

#endif
