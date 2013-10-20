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
                                                , pModel_( new QStandardItemModel )
                                                , pMethod_( new adcontrols::PeakMethod ) 
                                                , pDelegate_( new TableDelegate )
{
    ui->setupUi(this);
    ui->timeEvents->setModel( pModel_.get() );
    ui->timeEvents->setItemDelegate( pDelegate_.get() );
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
    (void)config;
}

void
PeakMethodForm::OnInitialUpdate()
{
    setContents( *pMethod_ );
    do {
        QStandardItemModel& model = *pModel_;
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
	pm.appendMethod< adcontrols::PeakMethod >( *pMethod_ );
}

void
PeakMethodForm::setContents( const adcontrols::PeakMethod& method )
{
    ui->doubleSpinSlope->setValue( method.slope() );
    ui->doubleSpinMinWidth->setValue( method.minimumWidth() );
    ui->doubleSpinMinHeight->setValue( method.minimumHeight() );
    ui->doubleSpinMinArea->setValue( method.minimumArea() );
    ui->doubleSpinDrift->setValue( method.drift() );
    ui->doubleSpinDoublingTime->setValue( method.doubleWidthTime() );
    ui->comboBoxWidth->setCurrentIndex( 0 );
    ui->comboBoxPlate->setCurrentIndex( 0 );
    ui->comboBoxPharmacopoeia->setCurrentIndex( 0 );
}

void qtwidgets::PeakMethodForm::on_doubleSpinSlope_valueChanged(double arg1)
{
    pMethod_->slope( arg1 );
}

void qtwidgets::PeakMethodForm::on_doubleSpinMinWidth_valueChanged(double arg1)
{
    pMethod_->minimumWidth( arg1 );
}

void qtwidgets::PeakMethodForm::on_doubleSpinMinHeight_valueChanged(double arg1)
{
    pMethod_->minimumHeight( arg1 );
}

void qtwidgets::PeakMethodForm::on_doubleSpinDrift_valueChanged(double arg1)
{
    pMethod_->drift( arg1 );
}

void qtwidgets::PeakMethodForm::on_doubleSpinMinArea_valueChanged(double arg1)
{
    pMethod_->minimumArea( arg1 );
}

void qtwidgets::PeakMethodForm::on_doubleSpinDoublingTime_valueChanged(double arg1)
{
    pMethod_->doubleWidthTime( arg1 );
}

void qtwidgets::PeakMethodForm::on_comboBoxPharmacopoeia_currentIndexChanged(int index)
{
    using namespace adcontrols::chromatography;

    if ( index == 0 ) // not specified
        pMethod_->pharmacopoeia( ePHARMACOPOEIA_NotSpcified );
    else if ( index == 1 ) // usp
        pMethod_->pharmacopoeia( ePHARMACOPOEIA_USP );
    else if ( index == 2 ) // ep
        pMethod_->pharmacopoeia( ePHARMACOPOEIA_EP );
    else if ( index == 3 ) // jp
        pMethod_->pharmacopoeia( ePHARMACOPOEIA_JP );

    if ( pMethod_->pharmacopoeia() == ePHARMACOPOEIA_USP ) {
        pMethod_->theoreticalPlateMethod( ePeakWidth_Tangent );
    } else if ( pMethod_->pharmacopoeia() == ePHARMACOPOEIA_EP ) {
        pMethod_->theoreticalPlateMethod( ePeakWidth_HalfHeight );
        pMethod_->peakWidthMethod( ePeakWidth_HalfHeight );
    }

}

void qtwidgets::PeakMethodForm::on_comboBoxPlate_currentIndexChanged(int index)
{
    (void)index; // todo
}

void qtwidgets::PeakMethodForm::on_comboBoxWidth_currentIndexChanged(int index)
{
    (void)index; // todo
}
