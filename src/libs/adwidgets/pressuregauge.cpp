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

#include "pressuregauge.hpp"
#include <boost/numeric/interval.hpp>
#include <qDebug>
#include <qwt_thermo.h>
#include <qwt_scale_engine.h>
#include <qwt_transform.h>
#include <qwt_color_map.h>
#include <qwt_math.h>
#include <QGridLayout>
#include <QLabel>
#include <QPalette>

namespace adwidgets {

    class LogColorMap : public QwtLinearColorMap {
    public:
        LogColorMap( const QColor& from, const QColor& to ) : QwtLinearColorMap( from, to )
        {}
        QRgb rgb( const QwtInterval& interval, double value ) const {
            return QwtLinearColorMap::rgb( QwtInterval( std::log10( interval.minValue() ), std::log10( interval.maxValue() ) ), std::log10( value ) );
        }
    };

    class PressureGauge::impl {
    public:
        ~impl() {}
        impl( Qt::Orientation orientation ) : orientation_( orientation )
                                            , limits_( std::make_pair( 0.0, 101300 ) )
                                            , thermo_( new QwtThermo() ) {
        }
        
        Qt::Orientation orientation_;
        QLabel data_;
        QLabel title_;
        double value_;
        std::pair< double, double > limits_;
        QwtThermo * thermo_;

        void onCreate( PressureGauge * pThis ) {
            auto gridLayout = new QGridLayout( pThis );
            if ( orientation_ == Qt::Horizontal ) {
                gridLayout->setHorizontalSpacing( 6 );
                gridLayout->setVerticalSpacing( 2 );
            } else {
                gridLayout->setHorizontalSpacing( 2 );
                gridLayout->setVerticalSpacing( 2 );
            }
            onCreate( gridLayout, 0, 0, pThis );
        }

        void onCreate( QGridLayout * gridLayout, int row, int col, PressureGauge * ) {
            thermo_->setOrientation( orientation_ );
            thermo_->setScalePosition( orientation_ == Qt::Horizontal ? QwtThermo::LeadingScale : QwtThermo::TrailingScale );

            thermo_->setScaleEngine( new QwtLogScaleEngine );
            thermo_->setScaleMaxMinor( 10 );
            const double lower = 1.0e-9;
            const double upper = 1.0e4;   // 0.1atm
            thermo_->setScale( lower, upper );

            auto *colorMap = new LogColorMap( Qt::blue, Qt::red );
            const double width = std::log10( upper ) - std::log10( lower );

            // double a1 = (std::log10( 4.55678e-8 ) - std::log10( lower )) / width; // Under flow threshold
            double a2 = (std::log10( 2.0e-5 ) - std::log10( lower )) / width;
            double a3 = (std::log10( 1.0e-4 ) - std::log10( lower ) ) / width;
            
            // colorMap->addColorStop( a1, Qt::blue );
            colorMap->addColorStop( a2, Qt::cyan );
            colorMap->addColorStop( a3, Qt::yellow );
            thermo_->setColorMap( colorMap );

            //thermo_->setFillBrush( Qt::darkCyan );
            thermo_->setAlarmBrush( Qt::magenta );
            thermo_->setAlarmLevel( 1.0 );

            gridLayout->addWidget( &title_, row, col++ );
            gridLayout->addWidget( &data_, row, col++ );
            gridLayout->addWidget( thermo_, row, col++ );
        }

    };

}

using namespace adwidgets;

PressureGauge::~PressureGauge()
{
    delete impl_;
}

PressureGauge::PressureGauge(QWidget *parent) : QWidget(parent)
                                              , impl_( new impl( Qt::Horizontal ) )
{
    impl_->onCreate( this );
}

PressureGauge::PressureGauge( Qt::Orientation orientation
                              , QWidget *parent) : QWidget(parent)
                                                 , impl_( new impl( orientation ) )
{
    impl_->onCreate( this );
}

PressureGauge::PressureGauge( Qt::Orientation orientation
                              , QGridLayout * grid, int row, int col, QWidget *parent) : QWidget(parent)
                                                                                       , impl_( new impl( orientation ) )
{
    impl_->onCreate( grid, row, col, this );
}

void
PressureGauge::setTitle( const QString& title )
{
    impl_->title_.setText( title );
}

void
PressureGauge::setActualValue( double value )
{
    impl_->value_ = value;
    impl_->data_.setText( QString::number( value, 'e', 3 ) );
    impl_->thermo_->setValue( value );
}

double
PressureGauge::value() const
{
    return impl_->value_;
}

void
PressureGauge::setRange( double low, double high )
{
    impl_->limits_ = std::make_pair( low, high );
}
