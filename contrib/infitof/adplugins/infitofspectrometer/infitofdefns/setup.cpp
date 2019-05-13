/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "setup.hpp"
#include <multumcontrols/scanlaw.hpp>
#include <infitofdefns/method.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adportable/debug.hpp>

using namespace infitof;

setup::setup()
{
}

// static
void
setup::default_method( infitof::ControlMethod& m )
{
    using namespace adcontrols::metric;

    multumcontrols::infitof::ScanLaw scanLaw;

    infitof::IonSource_EI_Method ei = { 0, 0, 0, 0, 0, 0, 0, 0 };
    m.ionSource = ei;
    m.tof.isLinear = true;
    m.tof.numAverage = 500;
    m.tof.gain = 1;

    ADDEBUG() << "setup::default_method trigInterval: " << m.tof.trigInterval;

    m.tof.trigInterval = 1000; // (us)
    m.tof.linear_protocol.lower_mass = 10;
    m.tof.linear_protocol.upper_mass = 200;

    m.tof.linear_protocol.avgr_delay = scanLaw.getTime( m.tof.linear_protocol.lower_mass, 0 );
    m.tof.linear_protocol.avgr_duration
        = scanLaw.getTime( m.tof.linear_protocol.upper_mass, 0 ) - m.tof.linear_protocol.avgr_delay;

    m.tof.linear_protocol.inject.delay = scale_to_base( 0.0, micro ); // injection open at 100us advance
    m.tof.linear_protocol.inject.width = scale_to_base( 10.0, micro );

    m.tof.linear_protocol.pulser.delay = scale_to_base( 0.0, micro ); // AP240 To (time zero)
    m.tof.linear_protocol.pulser.width = scale_to_base( 10.0, micro ); // 100us duration

    for ( auto& gate : m.tof.linear_protocol.gate ) {
        gate.delay = 0;
        gate.width = scale_to_base( 10.0, micro );
    }

    m.tof.linear_protocol.exit.delay   = m.tof.linear_protocol.pulser.delay;
    m.tof.linear_protocol.exit.width   = scale_to_base( 100.0, micro );

    m.tof.nTurn = 0;
    //m.scanLaw.isEnable = false;
    //m.scanLaw.nTurn = 0;
    m.tof.protocols.push_back( m.tof.linear_protocol );
}
