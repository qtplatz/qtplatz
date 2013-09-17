/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#if defined _MSC_VER
# pragma warning( disable : 4996 )
#endif

#include "mass_calibrator.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adportable/polfit.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <cmath>
#include <sstream>
#include <chrono>

using namespace dataproc;

mass_calibrator::mass_calibrator()
{
}

mass_calibrator::mass_calibrator( const adcontrols::MSAssignedMasses& assigned )
{
    for ( auto it: assigned ) {
        if ( it.enable() )
            (*this) << std::make_pair( it.time(), it.exactMass() );
    }
}

mass_calibrator&
mass_calibrator::operator << ( const std::pair< double, double >& pair )
{
    times_.push_back( pair.first );          // time
    sqrtMz_.push_back( sqrt( pair.second ) ); // sqrt( mass )
	return *this;
}

bool
mass_calibrator::compute( adcontrols::MSCalibration& calib, int nterm )
{
	if ( nterm == 0 )
		return false;

    std::vector< double > coeffs;
    if ( times_.size() >= size_t( nterm ) &&
         adportable::polfit::fit( times_.data(), sqrtMz_.data(), times_.size(), nterm, coeffs ) ) { 
        // set polynomials to result
        calib.coeffs( coeffs );

        // set id
        const boost::uuids::uuid uuid = boost::uuids::random_generator()();
        ident_ = boost::lexical_cast< std::wstring >( uuid );
        calib.calibId( ident_ );

        // set date time
        boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time();
        calib.date( boost::lexical_cast< std::string >( pt ) );
        
        return true;
    }
    return false;
}

double
mass_calibrator::compute_mass( double time, const adcontrols::MSCalibration& calib )
{
    double msqr = adcontrols::MSCalibration::compute( calib.coeffs(), time );
    if ( msqr > 0.0 )
        return msqr * msqr;
    return -1; // error
}

