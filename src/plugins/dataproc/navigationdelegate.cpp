// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include "navigationdelegate.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/datafile.hpp>
#include <qtwrapper/qstring.hpp>
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