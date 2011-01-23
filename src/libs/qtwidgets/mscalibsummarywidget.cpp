// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "mscalibsummarywidget.h"
#include "mscalibsummarydelegate.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <adcontrols/massspectrum.h>
#include <adcontrols/msreferences.h>
#include <adcontrols/msreference.h>

using namespace qtwidgets;

MSCalibSummaryWidget::~MSCalibSummaryWidget()
{
}

MSCalibSummaryWidget::MSCalibSummaryWidget(QWidget *parent) : QTableView(parent)
                                                            , pModel_( new QStandardItemModel )
                                                            , pDelegate_( new MSCalibSummaryDelegate ) 
{
    this->setModel( pModel_.get() );
    this->setItemDelegate( pDelegate_.get() );
}

void
MSCalibSummaryWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSCalibSummaryWidget::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    QTableView& tableView = *this;

    QStandardItem * rootNode = model.invisibleRootItem();
    tableView.setRowHeight( 0, 7 );
    rootNode->setColumnCount( 10 );
    
    int col = 0;
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "m/z(observed)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "time(us)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "Intensity" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "m/z(exact)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "m/z(calibrated)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( col++, Qt::Horizontal, QObject::tr( "error(ppm)" ) );
}

void
MSCalibSummaryWidget::OnUpdate( boost::any& )
{
}

void
MSCalibSummaryWidget::OnFinalClose()
{
}


void
MSCalibSummaryWidget::setData( const adcontrols::MassSpectrum& ms )
{
    if ( ! ms.isCentroid() )
        return;

    const double * intensities = ms.getIntensityArray();
    const double * masses = ms.getMassArray();
    
}

void
MSCalibSummaryWidget::setData( const adcontrols::MSReferences& ref )
{
}

