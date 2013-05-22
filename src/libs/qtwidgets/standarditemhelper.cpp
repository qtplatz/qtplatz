/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "standarditemhelper.hpp"
#include <qtwrapper/qstring.hpp>
#include <QStandardItem>

using namespace qtwidgets;

StandardItemHelper::StandardItemHelper()
{
}

QStandardItem *
StandardItemHelper::appendRow( QStandardItem * parent, const char * label, bool editable )
{
    QStandardItem * item = new QStandardItem( label );
    if ( ! editable )
        item->setEditable( false );
    if ( parent )
        parent->appendRow( item );
    return item;
}

QStandardItem *
StandardItemHelper::appendRow( QStandardItem * parent, const std::wstring& label, bool editable )
{
    QStandardItem * item = new QStandardItem( qtwrapper::qstring::copy( label ) );
    if ( ! editable )
        item->setEditable( false );
    if ( parent )
        parent->appendRow( item );
    return item;
}

QStandardItem *
StandardItemHelper::appendRow( QStandardItem * parent, const char * label, const QVariant& qv )
{
    QStandardItem * item = new QStandardItem( label );
    item->setEditable( false );
    if ( parent ) {
        parent->appendRow( item );
        QStandardItemModel& model = *item->model();
        int row = item->row();
        int col = item->column();
        model.setData( model.index( row, col + 1, parent->index() ), qv );
    }
    return item;
}

