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
#include "scanlaw.hpp"
#include <adportable/binary_search.hpp>
#include <vector>
#include <string>

namespace adcontrols {

    using namespace metric;

    namespace detail {
        
        template< enum MSCalibration::ALGORITHM > struct compute_mass {
            
            template< class ScanLaw >
            double operator()( double /*time*/, int /*mode*/, const ScanLaw&, const MSCalibration& ) const {
                return 0;
            }
            
        };
        
        template<> struct compute_mass< MSCalibration::TIMESQUARED > {
            
            static double compute( double time, const MSCalibration& calib ) {
                double t = scale_to( calib.time_prefix(), time );
                if ( !calib.t0_coeffs().empty() )
                    t -= calib.t0_coeffs()[ 0 ];
                return calib.compute_mass( t );
            }

        };
            
        template<> struct compute_mass< MSCalibration::MULTITURN_NORMALIZED > {

            static double compute( double time, double fLength, const MSCalibration& calib ) {

                double tL = time / fLength;
                double mass = calib.compute_mass( scale_to( calib.time_prefix(), tL ) ); // first estimation

                for ( int i = 0; i < 2; ++i ) {
                    double t0 = scale_to_base( calib.compute( calib.t0_coeffs(), std::sqrt( mass ) ), calib.time_prefix() );
                    tL = ( time - t0 ) / fLength;
                    mass = calib.compute_mass( scale_to( calib.time_prefix(), tL ) );
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
        ComputeMass( const ScanLaw& slaw
                     , const adcontrols::MSCalibration& clb ) : scanLaw( slaw )
                                                              , calib( clb )
                                                              , algo( calib.algorithm() ) {
        }

        inline double getMass( double time, double fLength ) const {
            if ( algo == MSCalibration::MULTITURN_NORMALIZED )
                return detail::compute_mass< MSCalibration::MULTITURN_NORMALIZED >::compute( time, fLength, calib );
            return detail::compute_mass< MSCalibration::TIMESQUARED >::compute( time, calib );            
        }

        inline double operator()( double time, int mode ) const {
            return getMass( time, scanLaw.fLength( mode ) );
        }

        inline double getTime( double mass, double fLength ) const { 
            size_t tupper = static_cast< size_t >( scale_to_nano( scanLaw.getTime( mass + 50, fLength ) ) );
            size_t idx = adportable::lower_bound( 0, tupper, mass, [this,fLength](size_t pos){
                    // with g++ (4.7), use of [=] does not capture this pointer so that cause segmentation violation
                    // when call getMethod() defined above.  As workaround, implisit capture for this make it work.
                    // This problem does not exist on both Applie clang++ (Xcode5) and VS2012
                    double t = scale_to_base<double>( pos, nano ); // 1ns precision
					double m = this->getMass( t, fLength );
                    return m;
                } );
            return scale_to_base<double>( idx, nano );
        }
		double getTime( double mass, int mode ) const {
			return getTime( mass, scanLaw.fLength( mode ) );
		}
        
    };

}

