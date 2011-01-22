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

#include "centroidform.h"
#include "ui_centroidform.h"
#include <adcontrols/processmethod.h>
#include <adcontrols/centroidmethod.h>
#include <QStandardItemModel>
#include "centroiddelegate.h"
#include "standarditemhelper.h"
using namespace qtwidgets;

/////////////////////

CentroidForm::CentroidForm(QWidget *parent) : QWidget(parent)
                                            , ui(new Ui::CentroidForm)
                                            , pModel_( new QStandardItemModel )
                                            , pMethod_( new adcontrols::CentroidMethod ) 
                                            , pDelegate_( new CentroidDelegate ) 
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
}

CentroidForm::~CentroidForm()
{
    delete ui;
}

void
CentroidForm::OnCreate( const adportable::Configuration& config )
{
    config_ = config;
}

void
CentroidForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    adcontrols::CentroidMethod& method = *pMethod_;

    QStandardItem * rootNode = model.invisibleRootItem();

    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Centroid" );
    model.setHeaderData( 1, Qt::Horizontal, "" );

    QStandardItem * scanType =
        StandardItemHelper::appendRow( rootNode, "ScanType", qVariantFromValue( CentroidDelegate::PeakWidthMethod( method.peakWidthMethod() ) ) );

    do {
        StandardItemHelper::appendRow( scanType, "Peak Width [Da]" );
        model.insertColumn( 1, scanType->index() );
        model.setData( model.index( 0, 1, model.item( 0, 0 )->index() ), method.rsTofInDa() );

        StandardItemHelper::appendRow( scanType, "Proportional [ppm]", method.rsPropoInPpm() );
        StandardItemHelper::appendRow( scanType, "Constant [Da]", method.rsConstInDa() );
    } while(0);

    StandardItemHelper::appendRow( rootNode, "Area/Height", qVariantFromValue( CentroidDelegate::AreaHeight( method.centroidAreaIntensity() ) ) );
    StandardItemHelper::appendRow( rootNode, "Baseline Width [Da]", method.baselineWidth() );
    StandardItemHelper::appendRow( rootNode, "Peak Centroid Fraction [%]", method.peakCentroidFraction() * 100 );

    // update_model();
    
    //--------------
    ui->treeView->expand( scanType->index() );
    ui->treeView->setColumnWidth( 0, 200 );
}

void
CentroidForm::OnFinalClose()
{
}

///
void
CentroidForm::update_model()
{
    QStandardItemModel& model = *pModel_;
    const adcontrols::CentroidMethod& method = *pMethod_;

    model.setData( model.index( 0, 1), qVariantFromValue( CentroidDelegate::PeakWidthMethod( method.peakWidthMethod() ) ) );

    model.setData( model.index( 0, 1, model.item( 0, 0 )->index() ), method.rsTofInDa() );
    model.setData( model.index( 1, 1, model.item( 0, 0 )->index() ), method.rsPropoInPpm() );
    model.setData( model.index( 2, 1, model.item( 0, 0 )->index() ), method.rsConstInDa() );
    
    do {
        QStandardItem * item = model.itemFromIndex( model.index( 0, 0, model.item( 0, 0 )->index() ) );
        item->setEnabled( false );
        item = model.itemFromIndex( model.index( 0, 1, model.item( 0, 0 )->index() ) );
        item->setEnabled( false );
    } while(0);

    model.setData( model.index( 1, 1 ), qVariantFromValue( CentroidDelegate::AreaHeight( method.centroidAreaIntensity() ) ) );
    model.setData( model.index( 2, 1 ), method.baselineWidth() );
    model.setData( model.index( 3, 1 ), method.peakCentroidFraction() * 100 );
}

void
CentroidForm::update_data()
{
    QStandardItemModel& model = *pModel_;
    adcontrols::CentroidMethod& method = *pMethod_;

    method.peakWidthMethod( qVariantValue< CentroidDelegate::PeakWidthMethod >( model.index( 0, 1 ).data( Qt::EditRole ) ).methodValue() );

    method.rsTofInDa( model.index( 0, 1, model.item( 0, 0 )->index() ).data( Qt::EditRole ).toDouble() );
    method.rsPropoInPpm( model.index( 1, 1, model.item( 0, 0 )->index() ).data( Qt::EditRole ).toDouble() );
    method.rsConstInDa( model.index( 2, 1, model.item( 0, 0 )->index() ).data( Qt::EditRole ).toDouble() );

    CentroidDelegate::AreaHeight ah = qVariantValue<CentroidDelegate::AreaHeight>( model.index( 0, 1 ).data( Qt::EditRole ) );
    method.centroidAreaIntensity( ah.methodValue() );

    method.baselineWidth( model.index( 2, 1 ).data( Qt::EditRole ).toDouble() );
    method.peakCentroidFraction( model.index( 3, 1 ).data( Qt::EditRole ).toDouble() / 100.0 );
}

void
CentroidForm::getContents( adcontrols::ProcessMethod& pm )
{
    update_data();
    pm.appendMethod< adcontrols::CentroidMethod >( *pMethod_ );
}