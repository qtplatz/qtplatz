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

#include "mslockform.hpp"
#include "ui_mslockform.h"
#include "mslockdelegate.hpp"
#include "standarditemhelper.hpp"
#include <adportable/configuration.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <QStandardItemModel>

using namespace qtwidgets;

MSLockForm::MSLockForm(QWidget *parent) : QWidget(parent)
                                        , ui(new Ui::MSLockForm)
                                        , pModel_( new QStandardItemModel )
                                        , pConfig_( new adportable::Configuration )
                                        , pDelegate_( new MSLockDelegate )
                                        , pMethod_( new adcontrols::MSLockMethod ) 
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
}

MSLockForm::~MSLockForm()
{
    delete ui;
}

void
MSLockForm::OnCreate( const adportable::Configuration& config )
{
    *pConfig_ = config;
}

void
MSLockForm::OnInitialUpdate()
{
    QStandardItemModel& model = *pModel_;
    // adcontrols::CentroidMethod& method = *pMethod_;

    QStandardItem * rootNode = model.invisibleRootItem();

    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Lock mass" );
/*
    QStandardItem * scanType =
        StandardItemHelper::appendRow( rootNode, "Algorithm", qVariantFromValue( CentroidDelegate::PeakWidthMethod( method_->peakWidthMethod() ) ) );

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
*/
}

void
MSLockForm::OnFinalClose()
{
}

QSize
MSLockForm::sizeHint() const
{
    return QSize( 300, 250 );
}

void
MSLockForm::getContents( adcontrols::ProcessMethod& )
{
}

void
MSLockForm::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = static_cast< adplugin::LifeCycle *>(this);
}
