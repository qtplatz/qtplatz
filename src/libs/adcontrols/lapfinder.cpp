// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "lapfinder.hpp"
#include "scanlaw.hpp"

using namespace adcontrols;

lapFinder::lapFinder( const adcontrols::ScanLaw& law, double mass, int laps )
    : scanlaw_( law ), mass_( mass ), laps_( laps ), time_( law.getTime( mass, laps ) )
    , tlap_( scanlaw_.getTime( mass, 2 ) - scanlaw_.getTime( mass, 1 ) ) {
}

std::pair< int, double >
lapFinder::operator()( double mass ) const
{
    if ( laps_ > 0 && mass > 0.5 ) {
        int laps = laps_;
        if ( mass < mass_ ) {
            do {
                double time_p = time_ - (tlap_/2);
                double time = scanlaw_.getTime( mass, laps );
                if ( time > time_p )
                    return { laps, time };
            } while ( ++laps && laps < 1000 );
        } else {
            do {
                double time_n = time_ + (tlap_/2);
                double time = scanlaw_.getTime( mass, laps );
                if ( time < time_n )
                    return { laps, time };
            } while ( laps && --laps );
        }
    }
    return { 0, 0 };
}
