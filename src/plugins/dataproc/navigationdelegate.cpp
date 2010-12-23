//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "navigationdelegate.h"
#include "dataprocessor.h"
#include <adcontrols/datafile.h>
#include <qtwrapper/qstring.h>
#include <qdebug.h>

using namespace dataproc;

NavigationDelegate::NavigationDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

QWidget *
NavigationDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    return QItemDelegate::createEditor( parent, option, index );
}

void
NavigationDelegate::setEditorData( QWidget * editor, const QModelIndex& index ) const
{
    if ( qVariantCanConvert< portfolio::Folium >( index.data( Qt::UserRole + 1 ) ) ) {
        QItemDelegate::setEditorData( editor, index );
    } else {
        QItemDelegate::setEditorData( editor, index );
    }
}

void
NavigationDelegate::setModelData( QWidget * editor, QAbstractItemModel* model, QModelIndex& index ) const
{
    QItemDelegate::setModelData( editor,  model, index );
}

void
NavigationDelegate::paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QVariant data = index.data( Qt::UserRole + 1 );

    if ( qVariantCanConvert< Dataprocessor * >( data ) ) {

        Dataprocessor * processor = qVariantValue< Dataprocessor * >( data );
        if ( processor ) {
            adcontrols::datafile& file = processor->file();
            this->drawDisplay( painter, option, option.rect, qtwrapper::qstring( file.filename() ) );
        }

    } else if ( qVariantCanConvert< portfolio::Folder >( data ) ) {

        portfolio::Folder folder = qVariantValue< portfolio::Folder >( data );
        this->drawDisplay( painter, option, option.rect, qtwrapper::qstring( folder.name() ) );

    } else if ( qVariantCanConvert< portfolio::Folium >( data ) ) {

        portfolio::Folium folium = qVariantValue< portfolio::Folium >( data );
        this->drawDisplay( painter, option, option.rect, qtwrapper::qstring( folium.name() ) );

    } else {

        QItemDelegate::paint( painter, option, index );

    }
}

void
NavigationDelegate::updateEditorGeometry( QWidget * editor, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    Q_UNUSED( index );
    editor->setGeometry( option.rect );
}