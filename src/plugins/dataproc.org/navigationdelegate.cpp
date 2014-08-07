// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <QEvent>
#include <qlineedit.h>

Q_DECLARE_METATYPE( portfolio::Folium )
Q_DECLARE_METATYPE( portfolio::Folder )
Q_DECLARE_METATYPE( dataproc::Dataprocessor * )

using namespace dataproc;

NavigationDelegate::NavigationDelegate(QObject *parent) :  QItemDelegate(parent)
{
    qRegisterMetaType< portfolio::Folium >();
    //qRegisterMetaType< portfolio::Folder >();
    //qRegisterMetaType< dataproc::Dataprocessor * >();
}

void
NavigationDelegate::setEditorData( QWidget * editor, const QModelIndex& index ) const
{
    QItemDelegate::setEditorData( editor, index );
}

void
NavigationDelegate::setModelData( QWidget * editor, QAbstractItemModel* model, const QModelIndex& index ) const
{
    QVariant data = index.data( Qt::UserRole );
    if ( qVariantCanConvert< portfolio::Folium >( data ) ) {
        portfolio::Folium folium = qVariantValue< portfolio::Folium >( data );
        QString value = static_cast< QLineEdit * >( editor )->text();
        folium.name( value.toStdWString() );
    } else {
        QItemDelegate::setModelData( editor,  model, index );
    }
}

void
NavigationDelegate::paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
    QVariant data = index.data( Qt::UserRole );

    if ( qVariantCanConvert< Dataprocessor * >( data ) ) {

        if ( Dataprocessor * processor = qVariantValue< Dataprocessor * >( data ) ) {
            adcontrols::datafile& file = processor->file();
            drawDisplay( painter, option, option.rect, qtwrapper::qstring( file.filename() ) );
        }

    } else if ( qVariantCanConvert< portfolio::Folder >( data ) ) {
        
        portfolio::Folder folder = qVariantValue< portfolio::Folder >( data );
        drawDisplay( painter, option, option.rect, QString::fromStdWString( folder.name() ) );

    } else {

        QItemDelegate::paint( painter, option, index );
        return;

    }
}

bool
NavigationDelegate::editorEvent( QEvent * event
                                     , QAbstractItemModel * model
                                     , const QStyleOptionViewItem& option
                                     , const QModelIndex& index )
{
    bool res = QItemDelegate::editorEvent( event, model, option, index );
    if ( event->type() == QEvent::MouseButtonRelease && model->flags(index) & Qt::ItemIsUserCheckable ) {

        Qt::CheckState isChecked = static_cast< Qt::CheckState >( index.data( Qt::CheckStateRole ).toUInt() );
        emit checkStateChanged( index, isChecked );

    }
    return res;
}
