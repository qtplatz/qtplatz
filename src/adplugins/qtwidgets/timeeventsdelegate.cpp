/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "timeeventsdelegate.hpp"
#include <adcontrols/peakmethod.hpp>
#include <boost/format.hpp>
#include <QComboBox>

namespace qtwidgets { namespace internal {

        static const char * functions [] = {
            "Lock"
            , "Forced base"
            , "Shift base"
            , "V-to-V"
            , "Tailing"
            , "Leading"
            , "Shoulder"
            , "Negative peak"
            , "Negative lock"
            , "Horizontal base"
            , "Post horizontal base"
            , "Forced peak"
            , "Slope"
            , "Minimum width"
            , "Minimum height"
            , "Minimum area"
            , "Drift"
            , "Elmination"
            , "Manual"
        };
    }
}

#define countof(x) ( sizeof(x)/sizeof(x[0]) )

using namespace qtwidgets;

TimeEventsDelegate::TimeEventsDelegate(QObject *parent) :  QItemDelegate(parent)
{
}

QWidget *
TimeEventsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.column() == c_function ) {
        QComboBox * pCombo = new QComboBox( parent );
        QStringList list;
        for ( size_t i = 0; i < countof( internal::functions ); ++i )
            list << internal::functions[ i ];
        pCombo->addItems( list );
        return pCombo;
    } else {
        return QItemDelegate::createEditor( parent, option, index );
  }
}

void
TimeEventsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if ( index.column() == c_function ) {
        int idx = index.data().toInt() - 1;
        if ( size_t( idx ) < countof( internal::functions ) )
            drawDisplay( painter, option, option.rect, internal::functions[ idx ] );
        else
            drawDisplay( painter, option, option.rect, index.data().toString() );
    } else if ( index.column() == c_time ) {
        double value = index.data().toDouble();
        drawDisplay( painter, option, option.rect, ( boost::format( "%.4lf" ) % value ).str().c_str() );
    } else if ( index.column() == c_event_value ) {
        double value = index.data().toDouble();
        drawDisplay( painter, option, option.rect, ( boost::format( "%.3lf" ) % value ).str().c_str() );
    } else {
        QItemDelegate::paint( painter, option, index );
    }
}

void
TimeEventsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if ( index.column() == c_function ) {
        QComboBox * p = dynamic_cast< QComboBox * >( editor );
        int value = p->currentIndex();
        adcontrols::chromatography::ePeakEvent func = static_cast< adcontrols::chromatography::ePeakEvent >( value + 1 );
        model->setData( index, func );
        adcontrols::PeakMethod::TimedEvent event( 0, func );
        if ( event.isBool() ) {
			if ( model->index( index.row(), c_event_value ).data( Qt::EditRole ).type() != QVariant::Bool )
                model->setData( model->index( index.row(), c_event_value ), event.boolValue() );
        } else if ( event.isDouble() ) {
            if ( model->index( index.row(), c_event_value ).data( Qt::EditRole ).type() != QVariant::Double )
                model->setData( model->index( index.row(), c_event_value ), event.doubleValue() );
        }
    } else
        QItemDelegate::setModelData( editor, model, index );

    if ( index.row() == model->rowCount() - 1 )
        model->insertRow( index.row() + 1 ); // add last blank line
}
