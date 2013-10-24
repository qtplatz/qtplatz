// This is a -*- C++ -*- header.
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

#pragma once

#include "adcontrols_global.h"
#include "mscalibration.hpp"
#include "metric/prefix.hpp"
#include <boost/variant.hpp>
#include <vector>
#include <string>

namespace adcontrols {

    using namespace metric;

    namespace detail {
        
        template< enum MSCalibration::ALGORITHM > struct compute_mass {
            
            template< class ScanLaw >
            double operator()( double time, int mode, const ScanLaw& law, const MSCalibration& calib ) const {
                return 0;
            }
            
        };
        
        template<> struct compute_mass< MSCalibration::TIMESQUARED > {
            
            template< class ScanLaw >
            static double compute( double time, int /*mode*/, const ScanLaw&, const MSCalibration& calib ) {
                double t = scale_to( calib.time_prefix(), time );
                if ( !calib.t0_coeffs().empty() )
                    t -= calib.t0_coeffs()[ 0 ];
                return calib.compute_mass( t );
            }
        };
            
        template<> struct compute_mass< MSCalibration::MULTITURN_NORMALIZED > {

            template< class ScanLaw >                
            static double compute( double time, int mode, const ScanLaw& law, const MSCalibration& calib ) {
                
                double t0 = 0;
                if ( calib.t0_coeffs().empty() )
                    return calib.compute_mass( scale_to( calib.time_prefix(), time - law.getTime(0, mode) ) / law.fLength( mode ) );
                
                double mass = law.getMass( time, mode );
                for ( int i = 0; i < 2; ++i ) {
                    t0 = scale_to_base( calib.compute( calib.t0_coeffs(), std::sqrt( mass ) ), calib.time_prefix() );
                    double T  = scale_to( calib.time_prefix(), time - t0 );
                    mass = calib.compute_mass( T / law.fLength( mode ) );
                }
                return mass;
            }
        };
        
    }

    template<class ScanLaw> class ComputeMass {
        const ScanLaw& scanLaw;
        const MSCalibration& calib;
        MSCalibration::ALGORITHM algo;
    public:
        ComputeMass( const ScanLaw& _scanLaw
                     , const adcontrols::MSCalibration& _calib) : scanLaw( _scanLaw )
                                                                , calib( _calib )
                                                                , algo( calib.algorithm() ) {
        }

        double operator()( double time, int mode ) const {
            if ( algo == MSCalibration::MULTITURN_NORMALIZED )
                return detail::compute_mass< MSCalibration::MULTITURN_NORMALIZED >::compute( time, mode, scanLaw, calib );
            return detail::compute_mass< MSCalibration::TIMESQUARED >::compute( time, mode, scanLaw, calib );            
        }
        
    };

}

