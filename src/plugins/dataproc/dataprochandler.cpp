// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include "mass_calibrator.hpp"
#include <adcontrols/annotations.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/computemass.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/tableofelement.hpp>
#include <adcontrols/targeting.hpp>
#include <adlog/logger.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>
#include <adportable/polfit.hpp>
#include <adportfolio/folium.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <chromatogr/chromatography.hpp>

#include <iomanip>
#include <fstream>
#include <adportable/string.hpp>
#include <boost/format.hpp>
//#include <boost/bind.hpp>

using namespace dataproc;

DataprocHandler::DataprocHandler()
{
}

bool
DataprocHandler::doCentroid( adcontrols::MSPeakInfo& pkInfo
                             , adcontrols::MassSpectrum& res
                             , const adcontrols::MassSpectrum& profile
                             , const adcontrols::CentroidMethod& m )
{
    return adprocessor::dataprocessor::doCentroid( pkInfo, res, profile, m );
}

bool
DataprocHandler::doIsotope( adcontrols::MassSpectrum&, const adcontrols::IsotopeMethod& )
{
    adcontrols::ChemicalFormula chemicalFormula;
#if 0
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
#endif
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

// static
bool
DataprocHandler::doAnnotateAssignedPeaks( adcontrols::MassSpectrum& centroid
                                          , const adcontrols::MSAssignedMasses& assignedMasses )
{
	adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( centroid );
    for ( auto& ms : segments )
        ms.set_annotations( adcontrols::annotations() ); // clear all

    for ( const auto& assigned: assignedMasses ) {
        adcontrols::MassSpectrum& ms = segments[ assigned.idMassSpectrum() ];
        size_t idx = assigned.idPeak();
		if ( assigned.enable() )
			ms.setColor( idx, 1 ); // red
		else
			ms.setColor( idx, 6 ); // dark red
        std::string text = adcontrols::ChemicalFormula::formatFormula( assigned.formula() );
        adcontrols::annotation anno( text, ms.mass( idx ),  ms.intensity( idx ), static_cast< int >(idx) );
        ms.addAnnotation( std::move( anno ) );
    }
	return true;
}

bool
DataprocHandler::doMSCalibration( adcontrols::MSCalibrateResult& res
                                  , adcontrols::MassSpectrum& centroid
                                  , const adcontrols::MSCalibrateMethod& m
                                  , const boost::uuids::uuid& clsid )
{
    using adcontrols::MSProperty;

    if ( auto calib = centroid.calibration() ) {
        res.setCalibration( *calib );
    }

    res.references( m.references() );
    double tolerance = m.massToleranceDa();
    double threshold = adcontrols::segments_helper::max_intensity( centroid ) * m.minimumRAPercent() / 100;
    res.tolerance( tolerance );
    res.threshold( threshold );

    assign_masses assigner( tolerance, threshold );

    adcontrols::MSAssignedMasses assignedMasses;

	adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( centroid );
    int n = 0;
	for ( auto seg: segments )
		assigner( assignedMasses, seg, res.references(), seg.mode(), n++ );

	res.assignedMasses( assignedMasses ); // set peak assign result

    // annotate each peak on spectrum
    doAnnotateAssignedPeaks( centroid, assignedMasses );

    mass_calibrator calibrator( assignedMasses, centroid.getMSProperty() );

    adcontrols::MSCalibration calib( clsid );

    if ( calibrator.polfit( calib, m.polynomialDegree() + 1 ) ) {
        for ( auto it: assignedMasses ) {
            double mass = calibrator.compute_mass( it.time(), it.mode(), calib );
            it.mass( mass );
        }

        res.setCalibration( calib );

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
                                  , const adcontrols::MSAssignedMasses& assigned
                                  , const boost::uuids::uuid& clsid )
{
    using adcontrols::MSProperty;

    const double tolerance = m.massToleranceDa();
    const double threshold = centroid.maxIntensity() * m.minimumRAPercent() / 100;
    res.tolerance( tolerance );  // set tolerance in result
    res.threshold( threshold );  // set threshold in result

    std::map< size_t, size_t > mode_map;
    for ( adcontrols::MSAssignedMasses::vector_type::const_iterator it = assigned.begin(); it != assigned.end(); ++it )
        mode_map[ it->mode() ]++;
    // std::map<size_t, size_t>::iterator itMax = std::max_element( mode_map.begin(), mode_map.end() );
    // int mode = static_cast<int>(itMax->first);

    mass_calibrator calibrator( assigned, centroid.getMSProperty() );

    adcontrols::MSCalibration calib( clsid );

    if ( ! calibrator.polfit( calib, m.polynomialDegree() + 1 ) )
        return false;

    res.references( m.references() );
    res.setCalibration( calib );
    centroid.setCalibration( calib, true ); // m/z assign based on manually determined peaks

    // continue auto-assign
    assign_masses assign( tolerance, threshold );
    adcontrols::MSAssignedMasses assignedMasses;
	adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( centroid );
	for ( size_t n = 0; n < segments.size(); ++n ) {
		assign( assignedMasses, segments[n], m.references(), 0, static_cast<int>(n) );
	}

    mass_calibrator calibrator2( assignedMasses, centroid.getMSProperty() );
    if ( calibrator2.polfit( calib, m.polynomialDegree() + 1 ) ) {
        for ( auto it: assignedMasses )
            it.mass( calibrator2.compute_mass( it.time(), it.mode(), calib ) );

        centroid.setCalibration( calib, true );
        res.setCalibration( calib );
        res.assignedMasses( assignedMasses );

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
        r.setIsCounting( c.isCounting() );
        return true;
    }
    return false;
}

//static
bool
DataprocHandler::apply_calibration( adcontrols::MassSpectrum& ms, const adcontrols::MSCalibration& calib )
{
    adcontrols::segment_wrapper<> segments( ms );

    for ( auto& fms: segments )
        fms.setCalibration( calib, true );

	return false;
}

//static
bool
DataprocHandler::reverse_copy( adcontrols::MSPeakInfo& pkinfo, const adcontrols::MassSpectrum& ms )
{
    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > ms_segs( ms );
	adcontrols::segment_wrapper< adcontrols::MSPeakInfo > pkinfo_segs( pkinfo );

    for ( size_t fcn = 0; fcn < ms_segs.size(); ++fcn ) {
        auto& fpk = pkinfo_segs[ fcn ];
        auto& fms = ms_segs[ fcn ];

        for ( size_t i = 0; i < fms.size(); ++i ) {
            auto pk = fpk.begin() + i;
            pk->assign_mass( fms.mass( i ) );
        }
    }
	return true;
}
