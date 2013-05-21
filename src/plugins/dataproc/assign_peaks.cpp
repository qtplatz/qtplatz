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

#include "assign_peaks.hpp"
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adportable/array_wrapper.hpp>
#include <cstring>

using namespace dataproc;


void
assign_peaks::operator () ( adcontrols::MSAssignedMasses& res
                            , const adcontrols::MassSpectrum& centroid, const adcontrols::MSAssignedMasses& assigned )
{
    adportable::array_wrapper< const double > times( centroid.getTimeArray(), centroid.size() );

    for ( adcontrols::MSAssignedMasses::vector_type::const_iterator it = assigned.begin(); it != assigned.end(); ++it ) {
        double t = it->time();
        adportable::array_wrapper< const double >::iterator pos = std::lower_bound( times.begin(), times.end(), t );
        if ( pos != times.end() ) {
            if ( pos != times.begin() ) {
                if ( std::abs( *pos - t ) > std::abs( *(pos - 1 ) - t ) )
                    --pos;
            }
            if ( std::abs( *pos - t ) < tolerance_ ) {
                adcontrols::MSAssignedMass a( *it );
                size_t idx = std::distance( times.begin(), pos );
                a.idMassSpectrum( idx );
                a.time( centroid.getTime( idx ) );
                a.mass( centroid.getMass( idx ) );
                res << a;
            }
        }
    }
}

int
assign_peaks::find_by_time( const adcontrols::MassSpectrum& centroid, double t, double tolerance )
{
    adportable::array_wrapper< const double > times( centroid.getTimeArray(), centroid.size() );
    adportable::array_wrapper< const double >::iterator pos = std::lower_bound( times.begin(), times.end(), t );
    if ( pos != times.end() ) {
        if ( pos != times.begin() ) {
            double a = std::abs( *pos - t );
            double b = std::abs( *(pos - 1) - t );
            if ( a > b )
                --pos;
        }
        if ( std::abs( *pos - t ) < tolerance )
            return std::distance( times.begin(), pos );
    }
    return -1;
}
