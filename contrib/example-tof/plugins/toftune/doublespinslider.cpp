/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "doublespinslider.hpp"
#include <QDoubleSpinBox>
#include <QSlider>
#include <QLabel>
#include <boost/format.hpp>

using namespace toftune;

DoubleSpinSlider::DoubleSpinSlider( QDoubleSpinBox * spin, QSlider * slider, double minimum, double maximum, QLabel * actual )
    : spin_( spin )
    , slider_( slider )
    , minimum_( minimum )
    , maximum_( maximum )
    , actual_( actual )
{
    slider_->setMinimum( minimum_ );
    slider_->setMaximum( maximum_ );
    spin_->setMinimum( minimum_ );
    spin_->setMaximum( maximum_ );
    if ( actual_ ) {
        QString format("<font color='%1'>--</font>");
        actual_->setText( format.arg( "gray" ) );
    }
    spin_->setKeyboardTracking( false );  // this will avoid valueChanged event for each key stroke
    connect( spin_, SIGNAL( valueChanged( double ) ), this, SLOT( onSpinValueChanged( double ) ) );
    connect( slider_, SIGNAL( valueChanged( int ) ), this, SLOT( onSliderValueChanged( int ) ) );
}

void
DoubleSpinSlider::onSpinValueChanged( double value )
{
    slider_->setValue( static_cast<int>( value ) );
    if ( actual_ ) {
        QString text = ( boost::format( "<font color='%1%'>%2%</font>" ) % "red" % value ).str().c_str();
        actual_->setText( text );
    }
    emit valueChanged();
}

void
DoubleSpinSlider::onSliderValueChanged( int value )
{
    spin_->setValue( static_cast<double>( value ) );
}

double
DoubleSpinSlider::value() const
{
   return spin_->value();
}

void
DoubleSpinSlider::value( double v )
{
    spin_->setValue( v );
}