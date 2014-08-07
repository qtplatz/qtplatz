/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
	if ( size_t nSize = centroid.size() ) {

		adportable::array_wrapper< const double > times( centroid.getTimeArray(), nSize );

		for ( auto it = assigned.begin(); it != assigned.end(); ++it ) {
			double t = it->time();
			auto pos = std::lower_bound( times.begin(), times.end(), t );
			if ( pos != times.end() ) {
				if ( pos != times.begin() ) {
					if ( std::abs( *pos - t ) > std::abs( *(pos - 1 ) - t ) )
						--pos;
				}
				if ( std::abs( *pos - t ) < tolerance_ ) {
					adcontrols::MSAssignedMass a( *it );
					size_t idx = std::distance( times.begin(), pos );
                    a.idMassSpectrum( uint32_t( idx ) );
                    a.time( centroid.getTime( uint32_t( idx ) ) );
                    a.mass( centroid.getMass( uint32_t( idx ) ) );
					res << a;
				}
			}
		}
	}
}

int
assign_peaks::find_by_time( const adcontrols::MassSpectrum& centroid, double t, double tolerance )
{
	if ( size_t nSize = centroid.size() ) {
	
		adportable::array_wrapper< const double > times( centroid.getTimeArray(), nSize );

		auto pos = std::lower_bound( times.begin(), times.end(), t );
		if ( pos != times.end() ) {
			if ( pos != times.begin() ) {
				double a = std::abs( *pos - t );
				double b = std::abs( *(pos - 1) - t );
				if ( a > b )
					--pos;
			}
			if ( std::abs( *pos - t ) < tolerance )
				return int( std::distance( times.begin(), pos ) );
		}
	}
    return -1;
}
