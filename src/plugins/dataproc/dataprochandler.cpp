// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include "assign_masses.hpp"
#include "calibrate_masses.hpp"
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
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/peakresult.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/polfit.hpp>
#include <adportable/debug.hpp>
#include <chromatogr/chromatography.hpp>

#include <iomanip>
#include <fstream>
#include <adportable/string.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>

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
    bool result = false;

    if ( peak_detector( m, profile ) )
        result = peak_detector.getCentroidSpectrum( res );

    if ( profile.numSegments() > 0 ) {
        for ( size_t fcn = 0; fcn < profile.numSegments(); ++fcn ) {
            adcontrols::MassSpectrum centroid;
            if ( doCentroid( centroid, profile[ fcn ], m ) ) {
                res.addSegment( centroid );
                result = true;
            }
        }
    }
    return result;
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

#if defined DEBUG && 0
static void
calibresult_validation( const adcontrols::MSCalibrateResult& res
                        , const adcontrols::MassSpectrum& centroid
                        , double threshold )
{
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
}
#endif

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

    // mass_assign mass_assign( tolerance, threshold );
    assign_masses assigner( tolerance, threshold );
    adcontrols::MSAssignedMasses assignedMasses;
    assigner( assignedMasses, centroid, res.references(), 0 );

    calibrate_masses calibrator;//( centroid );
    // mass_calibrator calibrator( centroid );
    adcontrols::MSCalibration calib;
    if ( calibrator( assignedMasses, m.polynomialDegree() + 1, calib, 0 ) ) {
        calibrator.update( assignedMasses, calib );
        centroid.setCalibration( calib, true );
        // calibrator.update( centroid, calib );
        res.calibration( calib );
        res.assignedMasses( assignedMasses );

        std::vector< unsigned char > colors( centroid.size() );
        assign_masses::make_color_array( colors.data(), assignedMasses, centroid.size() );
        centroid.setColorArray( colors.data() );
#if defined _DEBUG && 0
        calibresult_validation( res, centroid, threshold );
#endif
        return true;
    }
    return false;
}

bool
DataprocHandler::doMSCalibration( adcontrols::MSCalibrateResult& res
                                  , adcontrols::MassSpectrum& centroid
                                  , const adcontrols::MSCalibrateMethod& m
                                  , const adcontrols::MSAssignedMasses& assigned )
{
    using adcontrols::MSProperty;

    const double tolerance = m.massToleranceDa();
    const double threshold = centroid.getMaxIntensity() * m.minimumRAPercent() / 100;
    res.tolerance( tolerance );  // set tolerance in result
    res.threshold( threshold );  // set threshold in result

    std::map< size_t, size_t > mode_map;
    for ( adcontrols::MSAssignedMasses::vector_type::const_iterator it = assigned.begin(); it != assigned.end(); ++it ) 
        mode_map[ it->mode() ]++;
    std::map<size_t, size_t>::iterator itMax = std::max_element( mode_map.begin(), mode_map.end() );
    int mode = itMax->first;

    calibrate_masses calibrator;
    adcontrols::MSCalibration calib;
    if ( ! calibrator( assigned, m.polynomialDegree() + 1, calib, mode ) )
        return false;

    res.references( m.references() );
    res.calibration( calib );
    centroid.setCalibration( calib, true ); // m/z assign based on manually determined peaks

    // continue auto-assign
    assign_masses assign( tolerance, threshold );
    adcontrols::MSAssignedMasses assignedMasses;
    assign( assignedMasses, centroid, m.references(), mode );

    // populate manually assigned peaks
    for ( adcontrols::MSAssignedMasses::vector_type::const_iterator it = assigned.begin(); it != assigned.end(); ++it ) {
        if ( it->flags() )
            assignedMasses << *it;
    }

    if ( calibrator( assignedMasses, m.polynomialDegree() + 1, calib, mode ) ) {
        calibrator.update( assignedMasses, calib );
        centroid.setCalibration( calib, true );
        res.calibration( calib );
        res.assignedMasses( assignedMasses );

        std::vector< unsigned char > colors( centroid.size() );
        assign_masses::make_color_array( colors.data(), assignedMasses, centroid.size() );
        centroid.setColorArray( colors.data() );
        return true;
    }
    return false;
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
