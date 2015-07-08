/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "ap240verticalform.hpp"
#include "ui_ap240verticalform.h"
#include <ap240/digitizer.hpp>
#include <QSignalBlocker>

static const std::vector< double > fullScaleList = { 5.0, 2.0, 1.0, 0.5, 0.2, 0.1, 0.05 };

ap240VerticalForm::ap240VerticalForm(QWidget *parent) :  QWidget(parent)
                                                      , ui(new Ui::ap240verticalform)
                                                      , channel_( 0 )
{
    ui->setupUi(this);

    connect( ui->doubleSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this](double d){
            emit valueChanged( idOffset, channel_, QVariant(d) );
        });

    connect( ui->comboBox_3, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [this] ( int index ) {
            if ( index < fullScaleList.size() )
                emit valueChanged( idFullScale, channel_, QVariant( fullScaleList[ index ] ) );
        });    
    
    connect( ui->comboBox_2, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [this] ( int index ) {
            emit valueChanged( idCoupling, channel_, QVariant(index) );
        });    
    connect( ui->comboBox, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), [this] ( int index ) {
            emit valueChanged( idBandWidth, channel_, QVariant(index) );
        } );
    connect( ui->checkBox, &QCheckBox::stateChanged, [this] ( int state ) {
            emit valueChanged( idInvertData, channel_, QVariant( state == Qt::Checked ) );
        });    
}

ap240VerticalForm::~ap240VerticalForm()
{
    delete ui;
}

void
ap240VerticalForm::setChannel( int ch )
{
    channel_ = ch;
    if ( channel_ == ( -1 ) )
        ui->checkBox->setVisible( false );
}

int
ap240VerticalForm::channel() const
{
    return channel_;
}

void
ap240VerticalForm::set( const ap240::method& m )
{
    const QSignalBlocker blocker( this );

    const ap240::method::vertical_method& t = ( channel_ == ( -1 ) ) ? m.ext_ : ( channel_ == 1 ) ? m.ch1_ : m.ch2_;

    auto it = std::lower_bound( fullScaleList.begin(), fullScaleList.end(), t.fullScale, [] ( double a, double b ) { return a > b; } );
    if ( it != fullScaleList.end() ) {
        auto index = std::distance( fullScaleList.begin(), it );
        ui->comboBox_3->setCurrentIndex( int( index ) );
    }
    ui->doubleSpinBox->setValue( t.offset );
    ui->comboBox_2->setCurrentIndex( t.coupling );
    ui->comboBox->setCurrentIndex( t.bandwidth );
    ui->checkBox->setChecked( t.invertData );
}

void
ap240VerticalForm::get( ap240::method& m ) const
{
    ap240::method::vertical_method& t = ( channel_ == ( -1 ) ) ? m.ext_ : ( channel_ == 1 ) ? m.ch1_ : m.ch2_;

    auto index = ui->comboBox_3->currentIndex();
    if ( index >= 0 && index < fullScaleList.size() )
        t.fullScale = fullScaleList[ index ];
    t.offset = ui->doubleSpinBox->value();
    t.coupling = ui->comboBox_2->currentIndex();
    t.bandwidth = ui->comboBox->currentIndex();
    t.invertData = ui->checkBox->isChecked();
}
