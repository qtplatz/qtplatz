/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "mschromatogramform.hpp"
#include "ui_mschromatogramform.h"
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/is_type.hpp>

using namespace adwidgets;

MSChromatogramForm::MSChromatogramForm(QWidget *parent) :  QWidget(parent),
                                                           ui(new Ui::MSChromatogramForm)
{
    ui->setupUi(this);
    ui->comboBox->addItems( QStringList() << tr( "Profile" ) << tr( "Centroid" ) );
}

MSChromatogramForm::~MSChromatogramForm()
{
    delete ui;
}

void
MSChromatogramForm::OnCreate( const adportable::Configuration& )
{
}

void
MSChromatogramForm::OnInitialUpdate()
{
    setContents( adcontrols::MSChromatogramMethod() );
}

void
MSChromatogramForm::OnFinalClose()
{
}

bool
MSChromatogramForm::getContents( boost::any& any ) const
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_pointer( any ) ) {
        if ( adcontrols::ProcessMethod* pm = boost::any_cast< adcontrols::ProcessMethod* >( any ) ) {
            adcontrols::MSChromatogramMethod m;
            getContents( m );
            pm->appendMethod< adcontrols::MSChromatogramMethod >( m );
            return true;
        }
    }
    return false;
}

bool
MSChromatogramForm::setContents( boost::any& any )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( any ) ) {
        const adcontrols::ProcessMethod& pm = boost::any_cast< adcontrols::ProcessMethod& >( any );
        if ( const auto t = pm.find< adcontrols::MSChromatogramMethod >() ) {
            setContents( *t );
            return true;
        }
    }
    return false;
}

void
MSChromatogramForm::getLifeCycle( adplugin::LifeCycle *& p )
{
    p = this;
}

void
MSChromatogramForm::setContents( const adcontrols::MSChromatogramMethod& m )
{
    ui->comboBox->setCurrentIndex( m.dataSource() );
    if ( m.width() == adcontrols::MSChromatogramMethod::widthInDa )
        ui->radioButton->setChecked( true );
    else
        ui->radioButton_2->setChecked( true );
    ui->doubleSpinBox->setValue( m.width( adcontrols::MSChromatogramMethod::widthInDa ) );
    ui->spinBox->setValue( m.width( adcontrols::MSChromatogramMethod::widthInRP ) );

    ui->checkBox_lower->setChecked( m.lower_limit() < 0 ? false : true );
    ui->checkBox_upper->setChecked( m.upper_limit() < 0 ? false : true );

    ui->doubleSpinBox_2->setEnabled( ui->checkBox_lower->isChecked() );
    ui->doubleSpinBox_3->setEnabled( ui->checkBox_upper->isChecked() );
    
    ui->doubleSpinBox_2->setValue( m.lower_limit() );
    ui->doubleSpinBox_3->setValue( m.upper_limit() );
}

void
MSChromatogramForm::getContents( adcontrols::MSChromatogramMethod& m ) const
{
    m.dataSource( static_cast<adcontrols::MSChromatogramMethod::DataSource>(ui->comboBox->currentIndex()) );
    if ( ui->radioButton->isChecked() )
        m.widthMethod( adcontrols::MSChromatogramMethod::widthInDa );
    else
        m.widthMethod( adcontrols::MSChromatogramMethod::widthInRP );

    m.width( ui->doubleSpinBox->value(), adcontrols::MSChromatogramMethod::widthInDa );
    m.width( ui->spinBox->value(), adcontrols::MSChromatogramMethod::widthInRP );

    m.lower_limit( ui->doubleSpinBox_2->value() );
    m.upper_limit( ui->doubleSpinBox_3->value() );
}


