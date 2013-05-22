/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "calibrate_masses.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adportable/polfit.hpp>
#include <vector>
#include <cmath>

using namespace dataproc;

calibrate_masses::calibrate_masses()
{
}

bool
calibrate_masses::operator () ( const adcontrols::MSAssignedMasses& assigned
                                , size_t nterm, adcontrols::MSCalibration& calib, int mode )
{
    std::vector<double> tmvec, msvec, coeffs;
    for ( adcontrols::MSAssignedMasses::vector_type::const_iterator it = assigned.begin(); it != assigned.end(); ++it ) {
        if ( it->enable() && it->mode() == mode ) {
            msvec.push_back( sqrt( it->exactMass() ) );
            tmvec.push_back( it->time() );
        }
    }
    if ( tmvec.size() >= nterm &&
         adportable::polfit::fit( &tmvec[0], &msvec[0], tmvec.size(), nterm, coeffs ) ) {
        calib.coeffs( coeffs );
        return true;
    }
    return false;

}

void
calibrate_masses::update( adcontrols::MSAssignedMasses& masses, const adcontrols::MSCalibration& calib )
{
    for ( adcontrols::MSAssignedMasses::vector_type::iterator it = masses.begin(); it != masses.end(); ++it ) {
        double t = it->time();
        double mq = adcontrols::MSCalibration::compute( calib.coeffs(), t );
        if ( mq > 0.0 )
            it->mass( mq * mq );
    }
}
