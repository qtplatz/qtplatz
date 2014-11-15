/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "peaktable.hpp"
#include <QStandardItemModel>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <qtwrapper/qstring.hpp>

using namespace adwidgets;

PeakTable::~PeakTable()
{
    delete model_;
}

PeakTable::PeakTable( QWidget *parent ) : TableView( parent )
                                        , model_( new QStandardItemModel )
{
    setModel( model_ );
}

void
PeakTable::OnCreate( const adportable::Configuration& )
{
}

void
PeakTable::OnInitialUpdate()
{
    QStandardItemModel& model = *model_;
    QTableView& tableView = *this;

    QStandardItem * rootNode = model.invisibleRootItem();
    tableView.setRowHeight( 0, 7 );
    rootNode->setColumnCount(10);
    int col = 0;
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("name") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Retention Time(min)") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Area") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Height") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("NTP") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Rs") );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr("Asymmetry") );
}

void
PeakTable::OnFinalClose()
{
}

bool
PeakTable::getContents( boost::any& ) const
{
    return false;
}

bool
PeakTable::setContents( boost::any& )
{
    return false;
}


void
PeakTable::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}

void
PeakTable::setData( const adcontrols::Peaks& peaks )
{
    QStandardItemModel& model = *model_;
    model.removeRows( 0, model.rowCount() );

    using namespace adcontrols;
    for ( Peaks::vector_type::const_iterator it = peaks.begin(); it != peaks.end(); ++it )
        add( *it );
}

void
PeakTable::setData( const adcontrols::PeakResult& result )
{
    setData( result.peaks() );
}

void
PeakTable::add( const adcontrols::Peak& peak )
{
    QStandardItemModel& model = *model_;

    int row = model.rowCount();
    model.appendRow( new QStandardItem( qtwrapper::qstring( peak.name() ) ) );
    model.setData( model.index( row, 1 ), static_cast<double>( adcontrols::timeutil::toMinutes( peak.peakTime() ) ) );
    model.setData( model.index( row, 2 ), peak.peakArea() );
    model.setData( model.index( row, 3 ), peak.peakHeight() );
    model.setData( model.index( row, 4 ), peak.theoreticalPlate().ntp() );
    model.setData( model.index( row, 5 ), peak.resolution().resolution() );
    model.setData( model.index( row, 6 ), peak.asymmetry().asymmetry() );
}
