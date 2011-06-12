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

#include "peakresultwidget.hpp"
#include <QStandardItemModel>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <qtwrapper/qstring.hpp>

using namespace qtwidgets;

PeakResultWidget::~PeakResultWidget()
{
}

PeakResultWidget::PeakResultWidget(QWidget *parent) : QTableView(parent)
                                                    , pModel_( new QStandardItemModel ) 
{
    setModel( pModel_.get() );
    OnInitialUpdate();
}

void
PeakResultWidget::OnCreate( const adportable::Configuration& )
{
}

void
PeakResultWidget::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
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
PeakResultWidget::OnFinalClose()
{
}

void
PeakResultWidget::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}

void
PeakResultWidget::setData( const adcontrols::Peaks& peaks )
{
    QStandardItemModel& model = *pModel_;
    model.removeRows( 0, model.rowCount() );

    using namespace adcontrols;
    for ( Peaks::vector_type::const_iterator it = peaks.begin(); it != peaks.end(); ++it )
        add( *it );
}

void
PeakResultWidget::setData( const adcontrols::Chromatogram& c )
{
    setData( c.peaks() );
}

void
PeakResultWidget::add( const adcontrols::Peak& peak )
{
    QStandardItemModel& model = *pModel_;

    size_t row = model.rowCount();
    model.appendRow( new QStandardItem( qtwrapper::qstring( peak.name() ) ) );
    model.setData( model.index( row, 1 ), static_cast<double>( adcontrols::timeutil::toMinutes( peak.peakTime() ) ) );
    model.setData( model.index( row, 2 ), peak.peakArea() );
    model.setData( model.index( row, 3 ), peak.peakHeight() );
    model.setData( model.index( row, 4 ), peak.theoreticalPlate().ntp() );
    model.setData( model.index( row, 5 ), peak.resolution().resolution() );
    model.setData( model.index( row, 6 ), peak.asymmetry().asymmetry() );
}
