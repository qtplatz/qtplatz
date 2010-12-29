// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

class QStandardItem;
class QVariant;

namespace qtwidgets {

    class StandardItemHelper {
    public:
        StandardItemHelper();

        static QStandardItem * appendRow( QStandardItem * parent, const char * label, bool editable = false );
        static QStandardItem * appendRow( QStandardItem * parent, const char * label, const QVariant& );

        template<class T> static QStandardItem * appendRow( QStandardItem * parent, const char * label, const T& value ) {
            QStandardItem * item = new QStandardItem( label );
            item->setEditable( false );
            if ( parent ) {
                parent->appendRow( item );
                QStandardItemModel& model = *item->model();
                if ( parent->columnCount() <= ( item->column() + 1 ) ) 
                    model.insertColumn( item->column() + 1, parent->index() );
                model.setData( model.index( item->row(), item->column() + 1, parent->index() ), value );
            }
            return item;
        }
    };

}

