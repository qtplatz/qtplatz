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
#include <QGridLayout>
#include <QLabel>

namespace adwidgets {

    class PressureGauge::impl {
    public:
        ~impl() {}
        impl( Qt::Orientation orientation ) : orientation_( orientation ) {
        }
        
        Qt::Orientation orientation_;
        QLabel data_;
        QLabel title_;
        double value_;

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
            gridLayout->addWidget( &title_, row, col++ );
            gridLayout->addWidget( &data_, row, col++ );
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
    impl_->data_.setText( QString::number( value, 'f', 3 ) );
}

double
PressureGauge::value() const
{
    return impl_->value_;
}
