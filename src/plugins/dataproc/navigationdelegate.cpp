// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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
#include <adportable/debug.hpp>
#include <adwidgets/standarditemhelper.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <qdebug.h>
#include <QEvent>
#include <QPainter>
#include <qlineedit.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_DECLARE_METATYPE( portfolio::Folium )
Q_DECLARE_METATYPE( portfolio::Folder )
// Q_DECLARE_METATYPE( dataproc::Dataprocessor * )
#endif

using namespace dataproc;

NavigationDelegate::NavigationDelegate(QObject *parent) :  QStyledItemDelegate(parent)
{
    qRegisterMetaType< portfolio::Folium >();
}

void
NavigationDelegate::setEditorData( QWidget * editor, const QModelIndex& index ) const
{
    ADDEBUG() << "---------- setEditorData -----------" << index.data( Qt::EditRole ).toString().toStdString();
    QStyledItemDelegate::setEditorData( editor, index );
}

void
NavigationDelegate::setModelData( QWidget * editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
    ADDEBUG() << "---------- setModelData -----------" << index.data( Qt::EditRole ).toString().toStdString();
    QVariant data = index.data( Qt::UserRole );
    if ( data.canConvert< portfolio::Folium >() ) {
        portfolio::Folium folium = data.value< portfolio::Folium >();
        QString value = static_cast< QLineEdit * >( editor )->text();
        folium.name( value.toStdWString() );
    } else {
        QStyledItemDelegate::setModelData( editor,  model, index );
    }
}

void
NavigationDelegate::paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QVariant data = index.data( Qt::UserRole );
    QStyleOptionViewItem opt(option);
    initStyleOption( &opt, index );

    if ( data.canConvert< Dataprocessor * >() ) {
        QStyledItemDelegate::paint( painter, opt, index );
    } else if ( data.canConvert< portfolio::Folder >() ) {
        portfolio::Folder folder = data.value< portfolio::Folder >();
        QStyledItemDelegate::paint( painter, opt, index );
    } else if ( data.canConvert< portfolio::Folium >() ) {
        painter->save();
        auto folium = data.value< portfolio::Folium >();

        if ( folium.attribute( "mslock" ) == "true" ) {
            painter->fillRect( opt.rect, QColor( 0xff, 0xff, 0x00, 0x40 ) ); // yellow
        }
        if ( folium.attribute( "mslock_external" ) == "true" ) {
            painter->fillRect( opt.rect, QColor( 0x6a, 0xff, 0x00, 0x40 ) );
        }
        if ( folium.attribute( "tag" ) == "red" ) {
            painter->fillRect( opt.rect, QColor( 0xff, 0x63, 0x47, 0x40 ) ); // tomato
        }
        if ( folium.attribute( "tag" ) == "blue" ) {
            painter->fillRect( opt.rect, QColor( 0xa7, 0xc7, 0xe7, 0x80 ) ); // pastel blue
        }
        if ( folium.attribute( "tag" ) == "green" ) {
            painter->fillRect( opt.rect, QColor( 0x00, 0x80, 0x00, 0x40 ) ); // dark-green
        }
        if ( folium.attribute( "remove" ) == "true" ) {
            painter->fillRect( opt.rect, QColor( 0xd3, 0xd3, 0xd3, 0x80 ) ); // gray
            painter->setPen( Qt::gray );
            painter->drawText( opt.rect, opt.displayAlignment, index.data().toString() );
        } else {
            QStyledItemDelegate::paint( painter, opt, index );
        }
        painter->restore();
    } else {
        QStyledItemDelegate::paint( painter, option, index );
        return;
    }
}

QWidget *
NavigationDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const
{
    return QStyledItemDelegate::createEditor( parent, option, index );
}
