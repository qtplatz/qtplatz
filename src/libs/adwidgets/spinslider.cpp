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

#include "spinslider.hpp"
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <tuple>

namespace adwidgets {
    
    class SpinSlider::impl {
    public:
        ~impl() {}
        impl( Qt::Orientation orientation ) : orientation_( orientation ){
            std::get<0>(d_).setObjectName( "header" );
            std::get<3>(d_).setObjectName( "actual" );
            std::get<3>(d_).setMinimumWidth(40);
        }

        Qt::Orientation orientation_;
        typedef std::tuple< QLabel, QDoubleSpinBox, QSlider, QLabel > data_type;
        data_type d_;
        QLabel title_;
        
        void onCreate( SpinSlider * pThis ) {

            auto gridLayout = new QGridLayout( pThis );

            if ( orientation_ == Qt::Horizontal ) {
                gridLayout->setHorizontalSpacing(6);
                gridLayout->setVerticalSpacing(2);
            } else {
                gridLayout->setHorizontalSpacing(2);
                gridLayout->setVerticalSpacing(2);
            }
            onCreate( gridLayout, 0, 0, pThis );
        }

        void onCreate( QGridLayout * gridLayout, int row, int col, SpinSlider * pThis ) {
            
            auto& spin = std::get<1>(d_);
            auto& slider = std::get<2>(d_);
            slider.setOrientation( orientation_ );
            slider.setFocusPolicy( Qt::StrongFocus );
            slider.setTickInterval( 10 );
            slider.setSingleStep( 1 );
            
            if ( orientation_ == Qt::Horizontal ) {
                
                gridLayout->addWidget( &std::get<0>(d_), row, col++ ); // heading
                gridLayout->addWidget( &std::get<1>(d_), row, col++ ); // spin
                gridLayout->addWidget( &std::get<2>(d_), row, col++ ); // slider
                gridLayout->addWidget( &std::get<3>(d_), row, col++ ); // actual
                ++row;

            } else {

                gridLayout->addWidget( &title_, row++, col ); // title with respect to DualSpinSlider
                gridLayout->addWidget( &std::get<0>(d_), row++, col ); // heading
                gridLayout->addWidget( &std::get<1>(d_), row++, col ); // spin
                gridLayout->addWidget( &std::get<2>(d_), row++, col ); // slider
                gridLayout->addWidget( &std::get<3>(d_), row++, col ); // actual
                ++col;
            }

            std::get<3>( d_ ).setText( QString::number( 0.0, 'f', 0 ) );
            gridLayout->setColumnStretch( 2, 1 ); // slider

            connect( &spin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged)
                     , pThis, &SpinSlider::handleSpinValueChanged );
            connect( &slider, &QSlider::valueChanged, pThis, &SpinSlider::handleSliderValueChanged );
        }
    };
}

using namespace adwidgets;

SpinSlider::~SpinSlider()
{
    delete impl_;
}

SpinSlider::SpinSlider(QWidget *parent) : QWidget(parent)
                                        , impl_( new impl( Qt::Horizontal ) )
{
    impl_->onCreate(this);
}

SpinSlider::SpinSlider(Qt::Orientation orientation
                       , QWidget *parent) : QWidget( parent )
                                          , impl_( new impl( orientation ) )
{
    impl_->onCreate(this);    
}

SpinSlider::SpinSlider(Qt::Orientation orientation
                       , QGridLayout * gridLayout, int row, int col
                       , QWidget *parent ) : QWidget( parent )
                                           , impl_( new impl( orientation ) )
{
    impl_->onCreate( gridLayout, row, col, this );
}

void
SpinSlider::setTitle( const QString& header )
{
    std::get<0>( impl_->d_ ).setText( header );
}

void
SpinSlider::setRange( const QPair< double, double >& range )
{
    auto& slider = std::get<2>(impl_->d_);
    slider.setMinimum( range.first );
    slider.setMaximum( range.second );

    auto& spin = std::get<1>(impl_->d_);
    spin.setMinimum( range.first );
    spin.setMaximum( range.second );
    spin.setKeyboardTracking( false ); // avoid valueChanged event for each key stroke
}

void
SpinSlider::setActualValue( double value )
{
    std::get<3>( impl_->d_ ).setText( QString::number( value, 'f', 1 ) );
}

void
SpinSlider::setValue( double value )
{
    std::get<1>(impl_->d_).setValue( value );
}

double
SpinSlider::value() const
{
    return std::get<1>(impl_->d_).value();
}

void
SpinSlider::handleSpinValueChanged( double value )
{
    auto& slider = std::get<2>(impl_->d_);
    slider.setValue( static_cast<int>( value ) );

    auto& actLabel = std::get<3>(impl_->d_);
    actLabel.setText( QString( "<font color=%1>%2</font>" ).arg( "gray", QString::number(value, 'f', 1) ) );
    
    emit valueChanged( value );
}

void
SpinSlider::handleSliderValueChanged( int value )
{
    auto& spin = std::get<1>(impl_->d_);
    spin.setValue( static_cast<double>( value ) );
}

QLabel *
SpinSlider::labelActual()
{
    return &std::get<3>(impl_->d_);
}

QLabel *
SpinSlider::labelHeader()
{
    return &std::get<0>(impl_->d_);
}
