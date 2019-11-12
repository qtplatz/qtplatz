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
#if defined _MSC_VER
# pragma warning( disable : 4996 )
#endif

#include "mass_calibrator.hpp"
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/polfit.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>
#include <cmath>
#include <sstream>
#include <vector>

using namespace dataproc;

mass_calibrator::mass_calibrator( const adcontrols::MSAssignedMasses& assigned
                                  , const adcontrols::MSProperty& prop ) //: scanLaw_( prop.scanLaw() )
{
    for ( auto it: assigned ) {
        if ( it.enable() ) {
            double t = it.time(); // / scanLaw_->fLength( it.mode() );  // time for 1mL
            times_.push_back( t );
            sqrtMz_.push_back( std::sqrt( it.exactMass() ) );
        }
    }
}

bool
mass_calibrator::polfit( adcontrols::MSCalibration& calib, int nterm )
{
	if ( nterm == 0 )
		return false;

    calib = adcontrols::MSCalibration( calib.massSpectrometerClsid() ); // reset date, calibrationUuid

    std::vector< double > coeffs;
    if ( times_.size() >= size_t( nterm ) &&
         adportable::polfit::fit( times_.data(), sqrtMz_.data(), times_.size(), nterm, coeffs ) ) {
        calib.setCoeffs( coeffs );
        return true;
    }
    return false;
}

double
mass_calibrator::compute_mass( double time, int mode, const adcontrols::MSCalibration& calib )
{
    return calib.compute_mass( time );
}
