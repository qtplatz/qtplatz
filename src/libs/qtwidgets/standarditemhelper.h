// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

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

