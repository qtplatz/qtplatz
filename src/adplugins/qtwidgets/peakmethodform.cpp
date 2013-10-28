/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "peakmethodform.hpp"
#include "ui_peakmethodform.h"
#include "tabledelegate.hpp"
#include "peakmethoddelegate.hpp"
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <QStandardItemModel>
#include "standarditemhelper.hpp"
#include <boost/format.hpp>
#include <qtwrapper/qstring.hpp>
#include <qdebug.h>

using namespace qtwidgets;

PeakMethodForm::PeakMethodForm(QWidget *parent) : QWidget(parent)
                                                , ui(new Ui::PeakMethodForm)
                                                , pMethod_( new adcontrols::PeakMethod ) 
                                                , pTimeEventsModel_( new QStandardItemModel )
                                                , pTimeEventsDelegate_( new TableDelegate )
                                                , pGlobalModel_( new QStandardItemModel )
                                                , pGlobalDelegate_( new PeakMethodDelegate )
{
    ui->setupUi(this);

    ui->timeEvents->setModel( pTimeEventsModel_.get() );
    ui->timeEvents->setItemDelegate( pTimeEventsDelegate_.get() );
    ui->timeEvents->verticalHeader()->setDefaultSectionSize( 18 );
    ui->treeView->setModel( pGlobalModel_.get() );
    ui->treeView->setItemDelegate( pGlobalDelegate_.get() );
}

PeakMethodForm::~PeakMethodForm()
{
    delete ui;
}

void
PeakMethodForm::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}

void
PeakMethodForm::OnCreate( const adportable::Configuration& config )
{
    (void)config;
}

void
PeakMethodForm::OnInitialUpdate()
{
    setContents( *pMethod_ );
    do {
        QStandardItemModel& model = *pTimeEventsModel_;
        QStandardItem * rootNode = model.invisibleRootItem();
        rootNode->setColumnCount(3);
        model.setHeaderData( 0, Qt::Horizontal, "Time(min)" );
        model.setHeaderData( 0, Qt::Horizontal, "Func" );
        model.setHeaderData( 0, Qt::Horizontal, "Value" );
        model.setRowCount( 1 );
    } while ( 0 );

    do {
        QStandardItemModel& model = *pGlobalModel_;

        model.setColumnCount( c_num_columns );
        model.setHeaderData( c_header, Qt::Horizontal, "Function" );
        model.setHeaderData( c_value, Qt::Horizontal, "Value" );

        model.setRowCount( r_num_rows );
        model.setData( model.index( r_slope,         c_header ), "Slope [&mu;V/min]" );
        model.setData( model.index( r_min_width,     c_header ), "Minimum width[min]" );
        model.setData( model.index( r_min_height,    c_header ), "Minimum height" );    
        model.setData( model.index( r_drift,         c_header ), "Drift [height/min]" );
        model.setData( model.index( r_min_area,      c_header ), "Minumum area[&mu;V&sdot;s]" );
        model.setData( model.index( r_doubling_time, c_header ), "Peak width doubling time[min]" );
        model.setData( model.index( r_void_time, c_header ),     "T<sub>0</sub>" );
        model.setData( model.index( r_pharmacopoeia, c_header ), "Pharmacopoeia" );

        setContents( *pMethod_ );

        for ( int row = 0; row < r_num_rows; ++row )
            model.item( row, c_header )->setEditable( false );

        ui->treeView->resizeColumnToContents( 0 );
        ui->treeView->resizeColumnToContents( 1 );

    } while (0);
}

void
PeakMethodForm::OnFinalClose()
{
}

bool
PeakMethodForm::getContents( boost::any& any ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( any ) ) {

        adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any );
        getContents( *pm );

        return true;
    }

    return false;
}

bool
PeakMethodForm::setContents( boost::any& any )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( any ) ) {

        adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
        if ( const adcontrols::PeakMethod *p = pm.find< adcontrols::PeakMethod >() ) {
            *pMethod_ = *p;
            setContents( *p );
            return true;
        }
    }
    return false;
}

void
PeakMethodForm::getContents( adcontrols::ProcessMethod& pm ) const
{
    getContents( *pMethod_ );
	pm.appendMethod< adcontrols::PeakMethod >( *pMethod_ );
}

void
PeakMethodForm::setContents( const adcontrols::PeakMethod& method )
{
    QStandardItemModel& model = *pGlobalModel_;
    
    model.setData( model.index( r_slope,         c_value ), method.slope() );
    model.setData( model.index( r_min_width,     c_value ), method.minimumWidth() ); 
    model.setData( model.index( r_min_height,    c_value ), method.minimumHeight() );
    model.setData( model.index( r_drift,         c_value ), method.drift() );
    model.setData( model.index( r_min_area,      c_value ), method.minimumArea() );
    model.setData( model.index( r_doubling_time, c_value ), method.doubleWidthTime() );
    model.setData( model.index( r_void_time,     c_value ), method.t0() );
    model.setData( model.index( r_pharmacopoeia, c_value ), method.pharmacopoeia() );
}

void
PeakMethodForm::getContents( adcontrols::PeakMethod& method ) const
{
    QStandardItemModel& model = *pGlobalModel_;
    
    method.slope( model.index( r_slope, c_value ).data( Qt::EditRole ).toDouble() );
    method.minimumWidth( model.index( r_min_width, c_value ).data( Qt::EditRole ).toDouble() );
    method.minimumHeight( model.index( r_min_height, c_value ).data( Qt::EditRole ).toDouble() );
    method.drift( model.index( r_drift, c_value ).data( Qt::EditRole ).toDouble() );
    method.minimumArea( model.index( r_min_area, c_value ).data( Qt::EditRole ).toDouble() );
    method.doubleWidthTime(model.index( r_doubling_time, c_value ).data( Qt::EditRole ).toDouble() );
    method.t0( model.index( r_void_time, c_value ).data( Qt::EditRole ).toDouble() );
    int value = model.index( r_pharmacopoeia, c_value ).data( Qt::EditRole ).toInt();
    method.pharmacopoeia( static_cast< adcontrols::chromatography::ePharmacopoeia >( value ) );

    using adcontrols::chromatography::ePHARMACOPOEIA_USP;
    using adcontrols::chromatography::ePHARMACOPOEIA_EP;
    using adcontrols::chromatography::ePHARMACOPOEIA_JP;
    using adcontrols::chromatography::ePeakWidth_Tangent;
    using adcontrols::chromatography::ePeakWidth_HalfHeight;

    if ( method.pharmacopoeia() == ePHARMACOPOEIA_USP ) {
        method.theoreticalPlateMethod( ePeakWidth_Tangent );
    } else if ( method.pharmacopoeia() == ePHARMACOPOEIA_EP ) {
        method.theoreticalPlateMethod( ePeakWidth_HalfHeight );
        method.peakWidthMethod( ePeakWidth_HalfHeight );
    } else if ( method.pharmacopoeia() == ePHARMACOPOEIA_JP ) {
        method.theoreticalPlateMethod( ePeakWidth_HalfHeight );
        method.peakWidthMethod( ePeakWidth_HalfHeight );
    }
}
