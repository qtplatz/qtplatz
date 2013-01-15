/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "peakmethodform.hpp"
#include "ui_peakmethodform.h"
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
						, pModel_( new QStandardItemModel )
						, pConfig_( new adportable::Configuration )
						, pMethod_( new adcontrols::PeakMethod ) 
						, pDelegate_( new PeakMethodDelegate )
{
    ui->setupUi(this);
    ui->treeView->setModel( pModel_.get() );
    ui->treeView->setItemDelegate( pDelegate_.get() );
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
    QStandardItemModel& model = *pModel_;
    adcontrols::PeakMethod& method = *pMethod_;

    QStandardItem * rootNode = model.invisibleRootItem();
    ui->treeView->setItemDelegate( pDelegate_.get() );

    rootNode->setColumnCount(2);
    model.setHeaderData( 0, Qt::Horizontal, "Peak Method" );
    for ( int i = 1; i < rootNode->columnCount(); ++i )
        model.setHeaderData( 1, Qt::Horizontal, "" );

//----
    StandardItemHelper::appendRow( rootNode, "Slope [uV/min]",       method.slope() );
    StandardItemHelper::appendRow( rootNode, "Minimum width[min]",   method.minimumWidth() );
    StandardItemHelper::appendRow( rootNode, "Minimum height[uV]",   method.minimumHeight() );
    StandardItemHelper::appendRow( rootNode, "Drift[uV/min]",        method.drift() );
    StandardItemHelper::appendRow( rootNode, "Minimum Area[uV*s]",   method.minimumArea() );
    StandardItemHelper::appendRow( rootNode, "Peak width doubling time[min]"
                                                                   , method.doubleWidthTime() );

	StandardItemHelper::appendRow( rootNode, "Pharmacopoeia", qVariantFromValue( PeakMethodDelegate::PharmacopoeiaEnum( method.pharmacopoeia() ) ) );

    StandardItemHelper::appendRow( rootNode, "Theoretical Plate calculation method"
		                                                           , method.theoreticalPlateMethod() );
    StandardItemHelper::appendRow( rootNode, "Peak width calculation method"
		                                                           , method.peakWidthMethod() );

	ui->treeView->setColumnWidth( 0, 240 );
	ui->treeView->setColumnWidth( 1, 100 );
}

void
PeakMethodForm::OnFinalClose()
{
}

bool
PeakMethodForm::getContents( boost::any& ) const
{
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
    QStandardItemModel& model = *pModel_;
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

