/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "contoursform.hpp"
#include "ui_contoursform.h"
#include <QSignalBlocker>
#include <limits>

ContoursForm::ContoursForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContoursForm)
{
    ui->setupUi(this);
    ui->spinBox_3->setRange( 0, 256 );
    ui->spinBox_4->setRange( 0, std::numeric_limits<int>::max() );
    ui->spinBox_5->setRange( 0, std::numeric_limits<int>::max() );
    ui->spinBox_5->setValue( std::numeric_limits<int>::max() );
    
    connect( ui->spinBox, static_cast< void(QSpinBox::*)(int) >(&QSpinBox::valueChanged)
             , this, [&]( int value ){ emit valueChanged( idBlurSize, value ); });
    connect( ui->spinBox_2, static_cast< void(QSpinBox::*)(int) >(&QSpinBox::valueChanged)
             , this, [&]( int value ){ emit valueChanged( idResize, value ); });
    connect( ui->spinBox_3, static_cast< void(QSpinBox::*)(int) >(&QSpinBox::valueChanged)
             , this, [&]( int value ){ emit valueChanged( idCannyThreshold, value ); });
    connect( ui->spinBox_4, static_cast< void(QSpinBox::*)(int) >(&QSpinBox::valueChanged)
             , this, [&]( int value ){ emit valueChanged( idMinSizeThreshold, value ); });
    connect( ui->spinBox_5, static_cast< void(QSpinBox::*)(int) >(&QSpinBox::valueChanged)
             , this, [&]( int value ){ emit valueChanged( idMaxSizeThreshold, value ); });        
}

ContoursForm::~ContoursForm()
{
    delete ui;
}

void
ContoursForm::setBlurSize( int value )
{
    QSignalBlocker( ui->spinBox );
    ui->spinBox->setValue( value );
}

void
ContoursForm::setResize( int value )
{
    QSignalBlocker( ui->spinBox_2 );
    ui->spinBox_2->setValue( value );
}

void
ContoursForm::setCannyThreshold( int value )
{
    QSignalBlocker( ui->spinBox_3 );
    ui->spinBox_3->setValue( value );
}

void
ContoursForm::setMinSizeThreshold( unsigned value )
{
    QSignalBlocker( ui->spinBox_4 );
    if ( value > static_cast< unsigned >( std::numeric_limits< int >::max() ) )
        value = std::numeric_limits< int >::max();    
    ui->spinBox_4->setValue( value );
}

void
ContoursForm::setMaxSizeThreshold( unsigned value )
{
    QSignalBlocker( ui->spinBox_5 );
    if ( value > static_cast< unsigned >( std::numeric_limits< int >::max() ) )
        value = std::numeric_limits< int >::max();
    ui->spinBox_5->setValue( value );
}

unsigned
ContoursForm::minSizeThreshold() const
{
    return ui->spinBox_4->value();
}

unsigned
ContoursForm::maxSizeThreshold() const
{
    return ui->spinBox_5->value();
}
