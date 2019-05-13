/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "method_workaround.hpp"
#include "method.hpp"
#include <method.hpp>
#include <adcontrols/tofprotocol.hpp>

using namespace infcontrols;

void
method_workaround::copy( multumcontrols::OrbitProtocol& d, const infitof::OrbitProtocol& s )
{
    d.lower_mass = s.lower_mass;
    d.upper_mass = s.upper_mass;
    d.avgr_delay = s.avgr_delay;
    d.avgr_duration = s.avgr_duration;
    d.pulser = multumcontrols::DelayMethod( s.pulser.delay, s.pulser.width );
    d.inject = multumcontrols::DelayMethod( s.inject.delay, s.inject.width );
    d.exit = multumcontrols::DelayMethod( s.exit.delay, s.exit.width );
    d.gate.clear();
    for ( auto& x : s.gate )
        d.gate.push_back( multumcontrols::DelayMethod( x.delay, x.width ) );

    d.external_adc_delay = multumcontrols::DelayMethod( s.external_adc_delay.delay, s.external_adc_delay.width );

    d.description() = s.description_;
    d.formulae() = s.formulae_;
    d.additionals().clear();
    for ( auto& x : s.additionals_ )
        d.additionals().push_back( std::make_pair( multumcontrols::OrbitProtocol::eItem( x.first ), x.second ) );
    d.nlaps() = s.nlaps_;
    d.reference() = s.reference_;
}

void
method_workaround::copy( infitof::OrbitProtocol& d, const multumcontrols::OrbitProtocol& s )
{
    d.lower_mass = s.lower_mass;
    d.upper_mass = s.upper_mass;

    d.avgr_delay = s.avgr_delay;
    d.avgr_duration = s.avgr_duration;

    d.pulser = infitof::DelayMethod( s.pulser.delay, s.pulser.width );
    d.inject = infitof::DelayMethod( s.inject.delay, s.inject.width );
    d.exit   = infitof::DelayMethod( s.exit.delay, s.exit.width );

    d.gate.clear();
    for ( auto& x : s.gate )
        d.gate.push_back( infitof::DelayMethod( x.delay, x.width ) );

    d.external_adc_delay = infitof::DelayMethod( s.external_adc_delay.delay, s.external_adc_delay.width );

    d.description_ = s.description();
    d.formulae_ = s.formulae();

    d.additionals_.clear();
    for ( auto& x : s.additionals() )
        d.additionals_.push_back( std::make_pair( infitof::OrbitProtocol::eItem( x.first ), x.second ) );

    d.nlaps_ = s.nlaps();
    d.reference_ = s.reference();

}
