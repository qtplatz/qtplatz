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
#include <QStandardItemModel>
#include "standarditemhelper.hpp"
#include <boost/format.hpp>
#include <qtwrapper/qstring.hpp>
#include <qdebug.h>

using namespace qtwidgets;

PeakMethodForm::PeakMethodForm(QWidget *parent) : QWidget(parent)
                                                , ui(new Ui::PeakMethodForm)
                                                , pGEModel_( new QStandardItemModel )
                                                , pTEModel_( new QStandardItemModel )
                                                , pConfig_( new adportable::Configuration )
                                                , pMethod_( new adcontrols::PeakMethod ) 
                                                , pPMDelegate_( new PeakMethodDelegate )
                                                , pTEDelegate_( new TableDelegate )
{
    ui->setupUi(this);

    ui->globalEvents->setModel( pGEModel_.get() );
    ui->globalEvents->setItemDelegate( pPMDelegate_.get() );
    ui->globalEvents->verticalHeader()->setDefaultSectionSize( 18 );

    ui->timeEvents->setModel( pTEModel_.get() );
    ui->timeEvents->setItemDelegate( pTEDelegate_.get() );
    ui->timeEvents->verticalHeader()->setDefaultSectionSize( 18 );
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
    *pConfig_ = config;
}

void
PeakMethodForm::OnInitialUpdate()
{
    adcontrols::PeakMethod& method = *pMethod_;

    do {
        QStandardItemModel& model = *pGEModel_;

        QStandardItem * rootNode = model.invisibleRootItem();
        rootNode->setColumnCount(2);
        model.setHeaderData( 0, Qt::Horizontal, "Peak Method" );
        for ( int i = 1; i < rootNode->columnCount(); ++i )
            model.setHeaderData( i, Qt::Horizontal, "" );

        StandardItemHelper::appendRow( rootNode, "Slope [uV/min]",       method.slope() );
        StandardItemHelper::appendRow( rootNode, "Minimum width[min]",   method.minimumWidth() );
        StandardItemHelper::appendRow( rootNode, "Minimum height[uV]",   method.minimumHeight() );
        StandardItemHelper::appendRow( rootNode, "Drift[uV/min]",        method.drift() );
        StandardItemHelper::appendRow( rootNode, "Minimum Area[uV*s]",   method.minimumArea() );
        StandardItemHelper::appendRow( rootNode, "Peak width doubling time[min]"
                                       , method.doubleWidthTime() );
        StandardItemHelper::appendRow( rootNode, "Pharmacopoeia"
                                       , qVariantFromValue( PeakMethodDelegate::PharmacopoeiaEnum( method.pharmacopoeia() ) ) );
        
        StandardItemHelper::appendRow( rootNode, "Theoretical Plate calculation method"
                                       , method.theoreticalPlateMethod() );
        StandardItemHelper::appendRow( rootNode, "Peak width calculation method"
                                       , method.peakWidthMethod() );
        
        ui->globalEvents->setColumnWidth( 0, 240 );
        ui->globalEvents->setColumnWidth( 1, 100 );
    } while ( 0 );
    
    do {
        QStandardItemModel& model = *pTEModel_;
        QStandardItem * rootNode = model.invisibleRootItem();
        rootNode->setColumnCount(3);
        model.setHeaderData( 0, Qt::Horizontal, "Time(min)" );
        model.setHeaderData( 0, Qt::Horizontal, "Func" );
        model.setHeaderData( 0, Qt::Horizontal, "Value" );
        model.setRowCount( 1 );
    } while ( 0 );
}

void
PeakMethodForm::OnFinalClose()
{
}

bool
PeakMethodForm::getContents( boost::any& any ) const
{
	if ( any.type() != typeid( adcontrols::ProcessMethod* ) )
		return false;
	adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any );
	pm->appendMethod< adcontrols::PeakMethod >( *pMethod_ );
    return false;
}

bool
PeakMethodForm::setContents( boost::any& )
{
    return false;
}

void
PeakMethodForm::getContents( adcontrols::ProcessMethod& pm )
{
    QStandardItemModel& model = *pGEModel_;
    QStandardItem * root = model.invisibleRootItem();
    (void)root;
/*
    QVariant v = model.index( 0, 1, root->index() ).data( Qt::EditRole );
    pMethod_->polynomialDegree( v.toInt() );
    v = model.index( 1, 1, root->index() ).data( Qt::EditRole  );
    pMethod_->massToleranceDa( v.toDouble() );
    v = model.index( 2, 1, root->index() ).data( Qt::EditRole  );
    pMethod_->minimumRAPercent( v.toDouble() );
    v = model.index( 3, 1, root->index() ).data( Qt::EditRole  );
    pMethod_->lowMass( v.toDouble() );
    v = model.index( 4, 1, root->index() ).data( Qt::EditRole  );
    pMethod_->highMass( v.toDouble() );
*/
	pm.appendMethod< adcontrols::PeakMethod >( *pMethod_ );
}

