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

#include "dualspinslider.hpp"
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSlider>
#include <memory>
#include <array>
#include <tuple>

namespace adwidgets {

    class DualSpinSlider::impl {
    public:
        ~impl() {}

        impl( Qt::Orientation orientation ) : orientation_( orientation ) {
            std::get<0>(d_[0]).setObjectName( "header1" );
            std::get<0>(d_[1]).setObjectName( "header2" );
            std::get<3>(d_[0]).setObjectName( "actual1" );
            std::get<3>(d_[1]).setObjectName( "actual2" );
            for ( auto& d: d_ )
                std::get<3>(d).setMinimumWidth(40);
        }

        struct spinConnector {
            int idx;
            DualSpinSlider * pThis;
            QSlider * slider;
            spinConnector( DualSpinSlider * p, QSlider * s, int i ) : idx( i ), pThis( p ), slider(s) {}
            void operator()( double value ) {
                slider->setValue( int(value) );

                auto& label = std::get<3>( pThis->impl_->d_[idx] );
                label.setText( QString( "<font color=%1>%2</font>" ).arg( "gray", QString::number(value, 'f', 0) ) );

                emit pThis->valueChanged( idx, value );
            }
        };

        Qt::Orientation orientation_;
        typedef std::tuple< QLabel, QDoubleSpinBox, QSlider, QLabel > data_type;
        std::array< data_type, 2 > d_;
        std::vector< std::shared_ptr< spinConnector > > x_;
        QLabel title_;

        void onCreate( DualSpinSlider * pThis ) {

            auto gridLayout = new QGridLayout( pThis );
            
            for ( auto& d: d_ ) {
                
                auto& slider = std::get<2>(d);
                slider.setOrientation( orientation_ );
                slider.setFocusPolicy( Qt::StrongFocus );
                slider.setTickInterval( 10 );
                slider.setSingleStep( 1 );

            }

            if ( orientation_ == Qt::Horizontal ) {

                gridLayout->setHorizontalSpacing(6);
                gridLayout->setVerticalSpacing(2);

                gridLayout->addWidget( &title_, 0, 0, 1, 2 );  // row = 0, col = 0, rowspan = 1, colspan = 2

                int row = 1;
                for ( auto& d: d_ ) {
                    int col = 0;
                    gridLayout->addWidget( &std::get<0>(d), row, col++ ); // heading
                    gridLayout->addWidget( &std::get<1>(d), row, col++ ); // spin
                    gridLayout->addWidget( &std::get<2>(d), row, col++ ); // slider
                    gridLayout->addWidget( &std::get<3>(d), row, col++ ); // actual
                    ++row;
                }

            } else {
                gridLayout->setHorizontalSpacing(2);
                gridLayout->setVerticalSpacing(2);

                gridLayout->addWidget( &title_, 0, 0, 1, 2 );  // row = 0, col = 0, rowspan = 1, colspan = 2

                int col = 0;
                for ( auto& d: d_ ) {
                    int row = 1;
                    gridLayout->addWidget( &std::get<0>(d), row++, col ); // heading
                    gridLayout->addWidget( &std::get<1>(d), row++, col ); // spin
                    gridLayout->addWidget( &std::get<2>(d), row++, col ); // slider
                    gridLayout->addWidget( &std::get<3>(d), row++, col ); // actual
                    ++col;
                }

            }

            std::get<3>( d_[ 0 ] ).setText( QString::number( 0.0, 'f', 1 ) );
            std::get<3>( d_[ 1 ] ).setText( QString::number( 0.0, 'f', 1 ) );

            for ( int idx = 0; idx < 2; ++idx ) {

                auto& spin = std::get<1>( d_[ idx ] );
                auto& slider = std::get<2>( d_[ idx ] );
                
                auto x = std::make_shared< spinConnector >( pThis, &slider, idx );
                x_.push_back(x);
                
                connect( &spin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), *x );
                connect( &slider, &QSlider::valueChanged, [&](int value){ spin.setValue( value ); } );
            }

        }
    };
}

using namespace adwidgets;

DualSpinSlider::~DualSpinSlider()
{
    delete impl_;
}

DualSpinSlider::DualSpinSlider(QWidget *parent) : QWidget(parent)
                                                , impl_( new impl( Qt::Horizontal ) )
{
    impl_->onCreate(this);
}

DualSpinSlider::DualSpinSlider( Qt::Orientation orientation, QWidget *parent) : QWidget(parent)
                                                                              , impl_( new impl( orientation ) )
{
    impl_->onCreate(this);
}

void
DualSpinSlider::setTitles( const QString& title, const QStringList& headers )
{
    impl_->title_.setText( title );

    int idx = 0;
    for ( auto& header: headers )
        std::get<0>( impl_->d_[idx++] ).setText( header );
}

void
DualSpinSlider::setRange( idPair id, const QPair<double, double>& range )
{
    auto& spin = std::get<1>(impl_->d_[id]);
    auto& slider = std::get<2>(impl_->d_[id]);
    spin.setMinimum( range.first );
    spin.setMaximum( range.second );
    slider.setMinimum( range.first );
    slider.setMaximum( range.second );
}

void
DualSpinSlider::setActualValues( const QPair<double, double>& actuals )
{
    std::get<3>( impl_->d_[0] ).setText( QString::number( actuals.first, 'f', 1 ) );
    std::get<3>( impl_->d_[1] ).setText( QString::number( actuals.second, 'f', 1 ) );
}

void
DualSpinSlider::setActualValue( idPair idx, double value )
{
    std::get<3>( impl_->d_[idx] ).setText( QString::number( value, 'f', 1 ) );
}

void
DualSpinSlider::setValue( idPair idx, double value )
{
    std::get<1>( impl_->d_[idx] ).setValue( value );
}

void
DualSpinSlider::setValues( const QPair<double, double>& values )
{
    std::get<1>( impl_->d_[0] ).setValue( values.first );
    std::get<1>( impl_->d_[1] ).setValue( values.second );
}

double
DualSpinSlider::value( idPair idx ) const
{
    return std::get<1>( impl_->d_[idx] ).value();
}

QPair<double, double>
DualSpinSlider::values() const
{
    return QPair<double, double>( std::get<1>( impl_->d_[0] ).value(), std::get<1>( impl_->d_[1] ).value() );
}

