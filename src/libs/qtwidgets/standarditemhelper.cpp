//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "standarditemhelper.h"
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
StandardItemHelper::appendRow( QStandardItem * parent, const char * label, const QVariant& qv )
{
    QStandardItem * item = new QStandardItem( label );
    item->setEditable( false );
    if ( parent ) {
        parent->appendRow( item );
        QStandardItemModel& model = *item->model();
        model.setData( model.index( item->row(), item->column() + 1, parent->index() ), qv );
    }
    return item;
}
