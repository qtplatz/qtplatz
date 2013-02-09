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

#include "dataprochandler.hpp"
#include <adcontrols/centroidprocess.hpp>

#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/isotopemethod.hpp>

#include <adcontrols/tableofelements.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/peakresult.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/polfit.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/peakresult.hpp>
#include <chromatogr/chromatography.hpp>

//#ifdef _DEBUG
#include <iomanip>
#include <fstream>
#include <adportable/string.hpp>
#include <boost/format.hpp>
//#endif

using namespace dataproc;

DataprocHandler::DataprocHandler()
{
}

bool
DataprocHandler::doCentroid( adcontrols::MassSpectrum& res
                             , const adcontrols::MassSpectrum& profile
                             , const adcontrols::CentroidMethod& m )
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

namespace dataproc {

    struct mass_assign : boost::noncopyable {
        std::vector< std::pair< unsigned int, adcontrols::MSReference > > calibPoints;
        double tolerance;
        double RAThreshold;
        std::vector<unsigned char> colors;
        adcontrols::MSAssignedMasses assignedMasses;

        mass_assign( double _tolerance, double threshold ) : tolerance( _tolerance ), RAThreshold( threshold ) {
        }

        void operator()( const adcontrols::MassSpectrum& centroid, const adcontrols::MSReferences& references ) {
            using adportable::array_wrapper;
            using adcontrols::MSReferences;

            array_wrapper<const double> masses( centroid.getMassArray(), centroid.size() );
            array_wrapper<const double> intens( centroid.getIntensityArray(), centroid.size() );
            colors.resize( centroid.size() );
            memset( &colors[0], 0, centroid.size() * sizeof(unsigned char) );
            calibPoints.clear();

            size_t idReference(0);
            for ( MSReferences::vector_type::const_iterator it = references.begin(); it != references.end(); ++it ) {
                double exactMass = it->exactMass();
                array_wrapper<const double>::const_iterator lBound = std::lower_bound( masses.begin(), masses.end(), exactMass - tolerance );
                array_wrapper<const double>::const_iterator uBound = std::lower_bound( masses.begin(), masses.end(), exactMass + tolerance );

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
                    array_wrapper<const double>::const_iterator hIt = std::max_element( intens.begin() + lIdx, intens.begin() + uIdx );
                    if ( *hIt < RAThreshold )
                        continue;

                    size_t idx = std::distance( intens.begin(), hIt );
            
                    colors[ idx ] = it->enable() ? 1 : 0;
                    adcontrols::MSAssignedMass assigned( idReference, idx, it->formula(), it->exactMass(), centroid.getTime( idx ), masses[ idx ], it->enable() ); 
                    assignedMasses << assigned;
                    if ( it->enable() )
                        calibPoints.push_back( std::make_pair( idx, *it ) );
                }
                ++idReference;
            }
        }
    };

}

bool
DataprocHandler::doMSCalibration( adcontrols::MSCalibrateResult& res
                                 , adcontrols::MassSpectrum& centroid
                                 , const adcontrols::MSCalibrateMethod& m )
{
    using adcontrols::MSProperty;

    res.calibration( centroid.calibration() );
    res.references( m.references() );
    double tolerance = m.massToleranceDa();
    double threshold = centroid.getMaxIntensity() * m.minimumRAPercent() / 100;
    res.tolerance( tolerance );
    res.threshold( threshold );

    adportable::array_wrapper<const double> masses( centroid.getMassArray(), centroid.size() );
    adportable::array_wrapper<const double> intens( centroid.getIntensityArray(), centroid.size() );

    mass_assign mass_assign( tolerance, threshold );
    mass_assign( centroid, res.references() );
    res.assignedMasses( mass_assign.assignedMasses );

    if ( mass_assign.calibPoints.size() >= 2 ) {

        std::vector<double> tmvec, msvec, coeffs;
        for ( size_t i = 0; i < mass_assign.calibPoints.size(); ++i ) {
            msvec.push_back( sqrt( mass_assign.calibPoints[i].second.exactMass() ) );
            tmvec.push_back( centroid.getTime( mass_assign.calibPoints[i].first ) );
        }
        size_t nterm = m.polynomialDegree() + 1;
        if ( adportable::polfit::fit( &tmvec[0], &msvec[0], tmvec.size(), nterm, coeffs ) ) {
            adcontrols::MSCalibration calib;
            calib.coeffs( coeffs );
            res.calibration( calib );

            for ( adcontrols::MSAssignedMasses::vector_type::iterator it = res.assignedMasses().begin(); it != res.assignedMasses().end(); ++it ) {
                double t = it->time();
                double mq = adcontrols::MSCalibration::compute( coeffs, t );
                it->mass( mq * mq );
            }
        }
    }

    centroid.setColorArray( &mass_assign.colors[0] );
    do {
        if ( res.calibration().coeffs().size() >= 2 ) {
            const double * times = centroid.getTimeArray();
            for ( size_t i = 0; i < centroid.size(); ++i ) {
                double mq = adcontrols::MSCalibration::compute( res.calibration().coeffs(), times[i] );
                centroid.setMass( i, mq * mq );
            }
        }
    } while(0);
    

    //////////////////////
#if defined _DEBUG && 0
    do {
        const adcontrols::MSReferences& ref = res.references();
        const adcontrols::MSAssignedMasses& assigned = res.assignedMasses();

        std::ofstream of( "massassign.txt" );
        of << "#\tm/z(observed)\ttof(us)\tintensity\t\tformula,\tm/z(exact)\tm/z(calibrated)\terror(mDa)" << std::endl;

        adcontrols::MSReferences::vector_type::const_iterator refIt = ref.begin();
        for ( adcontrols::MSAssignedMasses::vector_type::const_iterator it = assigned.begin(); it != assigned.end(); ++it, ++refIt ) {
            const adcontrols::MSAssignedMass& a = *it;

            std::string formula = adportable::string::convert( a.formula() );
            of << std::setprecision(8)
                << std::setw(4) << a.idMassSpectrum() << "\t" // id
                << std::setw(15) << std::fixed << centroid.getMass( a.idMassSpectrum() ) << "\t"           // m/z(observed)
                << std::scientific << centroid.getTime( a.idMassSpectrum() ) << "\t"      // tof
                << std::fixed << std::setprecision( 0 ) << centroid.getIntensity( a.idMassSpectrum() ) << "\t" // intensity
                << formula << "\t"
                << std::setprecision(8) << std::fixed   << it->exactMass() << "\t"                             // mass(exact)
                << std::fixed   << a.mass() << "\t"                                    // m/z(calibrated)
                << std::setprecision(1) << ( a.mass() - it->exactMass() ) * 1000 << "\t"  // error(mDa)
                << ( it->enable() ? "used" : "not used" ) 
                << std::endl;
        }
        const std::vector<double>& coeffs = res.calibration().coeffs();

        of << "#--------------------------- Calibration coefficients: " << std::endl;
        for ( size_t i = 0; i < coeffs.size(); ++i )
            of << std::scientific << std::setprecision(14) << coeffs[i] << std::endl;
             
        of << "#--------------------------- centroid peak list (#,mass,intensity)--------------------------" << std::endl;

        adcontrols::MSReferences::vector_type::const_iterator it = res.references().begin();
        for ( size_t i = 0; i < centroid.size(); ++i ) {
            if ( centroid.getIntensity( i ) > threshold ) {
                
                double mq = adcontrols::MSCalibration::compute( res.calibration().coeffs(), centroid.getTime( i ) );
                double mass = mq * mq;

                double error = 0;
                if ( it != res.references().end() && std::abs( it->exactMass() - mass ) < 0.2 ) {
                    error = ( it->exactMass() - mass ) * 1000; // mDa
                    ++it;
                }
                of << i << "\t"
                    << std::setprecision(8) << std::fixed << centroid.getMass( i ) << "\t"
                    << std::setprecision(8) << mass << "\t"
                    << std::setprecision(1) << centroid.getIntensityArray()[i] << std::endl;
            }
        }
    } while(0);
#endif
    return true;
}

// static
bool
DataprocHandler::doFindPeaks( adcontrols::PeakResult& r, const adcontrols::Chromatogram& c, const adcontrols::PeakMethod& m )
{
	chromatogr::Chromatography peakfinder( m );

	if ( peakfinder( c ) ) {
        r.baselines() = peakfinder.getBaselines();
        r.peaks() = peakfinder.getPeaks();
		return true;
	}
	return false;
}
