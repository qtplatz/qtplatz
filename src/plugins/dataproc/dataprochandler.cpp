// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "dataprochandler.h"
#include <adcontrols/centroidprocess.h>

#include <adcontrols/isotopecluster.h>
#include <adcontrols/isotopemethod.h>

#include <adcontrols/tableofelements.h>
#include <adcontrols/chemicalformula.h>
#include <adcontrols/massspectrum.h>
#include <adcontrols/mscalibrateresult.h>
#include <adcontrols/mscalibratemethod.h>
#include <adportable/array_wrapper.hpp>
#include <adcontrols/msreferences.h>
#include <adcontrols/msreference.h>
#include <adcontrols/msassignedmass.h>
#include <adcontrols/mscalibration.h>

using namespace dataproc;

DataprocHandler::DataprocHandler()
{
}

bool
DataprocHandler::doCentroid( adcontrols::MassSpectrum& res, const adcontrols::MassSpectrum& profile, const adcontrols::CentroidMethod& m )
{
    adcontrols::CentroidProcess peak_detector;
    if ( peak_detector( m, profile ) )
        return peak_detector.getCentroidSpectrum( res );
    return false;
}

bool
DataprocHandler::doIsotope( adcontrols::MassSpectrum& res, const adcontrols::IsotopeMethod& m )
{
    adcontrols::ChemicalFormula chemicalFormula; 

    adcontrols::IsotopeCluster cluster;
    cluster.clearFormulae();

    double ra = res.getMaxIntensity();
    if ( ra <= 1.0 )
        ra = 100;

    if ( m.size() ) {

        for ( adcontrols::IsotopeMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it ) {
            std::wstring stdFormula = chemicalFormula.standardFormula( it->formula );
            cluster.addFormula( stdFormula, it->adduct, it->chargeState, it->relativeAmounts );
        }

        res.resize(0);  // clear peaks
        size_t nPeaks(0);
        if ( cluster.computeFormulae( m.threshold(), true, m.resolution(), res, nPeaks, m.useElectronMass(), ra / 100 ) ) {
            std::pair<double, double> mrange = res.getAcquisitionMassRange();
            if ( nPeaks && mrange.second <= 1.0 ) {
                mrange.second = res.getMassArray()[ res.size() - 1 ];
                mrange.first = res.getMassArray()[ 0 ];
                double d = mrange.second - mrange.first;
                mrange.first -= d / 10;
                mrange.second += d / 10;
                res.setAcquisitionMassRange( mrange.first, mrange.second );
            }
            return true;
        }

    }
    return false;
}

bool
DataprocHandler::doMSCalibration( adcontrols::MSCalibrateResult& res
                                 , const adcontrols::MassSpectrum& centroid
                                 , const adcontrols::MSCalibrateMethod& m )
{
    res.calibration( centroid.calibration() );
    res.references( m.references() );
    double tolerance = m.massToleranceDa();

    adportable::array_wrapper<const double> masses( centroid.getMassArray(), centroid.size() );
    adportable::array_wrapper<const double> intens( centroid.getIntensityArray(), centroid.size() );
    const double * times = centroid.getTimeArray();

    std::vector< unsigned char > colors( centroid.size() );
    memset( &colors[0], 0, colors.size() * sizeof( unsigned char ) );

    std::vector< std::pair< unsigned int, adcontrols::MSReference > > calibPoints;
    size_t idReference(0);
    for ( adcontrols::MSReferences::vector_type::const_iterator it = res.references().begin(); it != res.references().end(); ++it ) {
        const adcontrols::MSReference& ref = *it;
        double exactMass = ref.exactMass();
        adportable::array_wrapper<const double>::const_iterator lBound = std::lower_bound( masses.begin(), masses.end(), exactMass - tolerance );
        adportable::array_wrapper<const double>::const_iterator uBound = std::lower_bound( masses.begin(), masses.end(), exactMass + tolerance );

        if ( lBound != masses.end() ) {

            size_t lIdx = std::distance( masses.begin(), lBound );
            size_t uIdx = std::distance( masses.begin(), uBound );

            // find closest
            size_t cIdx = lIdx;
            for ( size_t i = lIdx + 1; i < uIdx; ++i ) {
                double d0 = std::abs( masses[ cIdx ] - exactMass );
                double d1 = std::abs( masses[ i ] - exactMass );
                if ( d1 < d0 )
                    cIdx = i;
            }

            // find highest
            adportable::array_wrapper<const double>::const_iterator hIt = std::max_element( intens.begin() + lIdx, intens.end() + uIdx );

            size_t idx = std::distance( intens.begin(), hIt );
            if ( idx != cIdx )
                idx = cIdx; // take closest, this is a workaround for Xe spectrum
            
            colors[ idx ] = ref.enable() ? 1 : 0;
            adcontrols::MSAssignedMass assigned( idReference, idx, it->formula(), it->exactMass(), times[ idx ], masses[ idx ] ); 
            res.assignedMasses() << assigned;
            if ( ref.enable() )
                calibPoints.push_back( std::make_pair( idx, *it ) );
        }
        ++idReference;
    }

    do {
        if ( calibPoints.size() == 2 ) {
            double m1 = calibPoints[0].second.exactMass();
            double m2 = calibPoints[1].second.exactMass();
            double t1 = times[ calibPoints[0].first ];
            double t2 = times[ calibPoints[1].first ];
            // theoretical calibration  [ sqrt(m) = a + b*t ]
            double b = ( std::sqrt( m2 ) - std::sqrt( m1 ) ) / ( t2 - t1 );
            double a = std::sqrt( m1 ) - b * t1;
            std::vector< double > coeffs;
            coeffs.push_back( a );
            coeffs.push_back( b );
            adcontrols::MSCalibration calib;
            calib.coeffs( coeffs );
            res.calibration( calib );

            // ------------
            for ( adcontrols::MSAssignedMasses::vector_type::iterator it = res.assignedMasses().begin(); it != res.assignedMasses().end(); ++it ) {
                double t = it->time();
                double mq = adcontrols::MSCalibration::compute( coeffs, t );
                it->mass( mq * mq );
            }
        }
    } while( 0 );


    const_cast< adcontrols::MassSpectrum& >( centroid ).setColorArray( &colors[0] );

    return true;
}

