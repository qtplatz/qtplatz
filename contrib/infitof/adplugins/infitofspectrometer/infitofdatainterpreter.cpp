/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "infitofdatainterpreter.hpp"
#include "constants.hpp"
#include "ap240translator.hpp"
#include "u5303a_translator.hpp"
#include "textfile.hpp"
#include <compiler/boost/workaround.hpp>
#include <infitofdefns/serializer.hpp>
#include <infitofdefns/avgrdata.hpp>
#include <infitofdefns/tracedata.hpp>
#include <multumcontrols/scanlaw.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>

#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <adcontrols/traceaccessor.hpp>

#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/date_string.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/waveform_peakfinder.hpp>
#include <boost/format.hpp>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

// for lockmass loggging
#include <fstream>
#include <adportable/profile.hpp>
#include <boost/filesystem/path.hpp>
#include <ctime>

#if defined DEBUG
# include <iostream>
#endif

using adcontrols::translate_state;

using namespace infitofspectrometer;

//////////////////////////////////////////////////////

InfiTofDataInterpreter::~InfiTofDataInterpreter()
{
}

InfiTofDataInterpreter::InfiTofDataInterpreter() : mslocker_( std::make_shared< adcontrols::lockmass::mslock >() )
                                                 , peakwidth_( 2.0e-9 ) // 2ns
                                                 , npeaksamples_( 5 )   // := 5pts
{
}

translate_state
InfiTofDataInterpreter::translate( adcontrols::MassSpectrum& xms
                                   , const char * data, size_t dsize
                                   , const char * meta, size_t msize
                                   , const adcontrols::MassSpectrometer& spectrometer
                                   , size_t idData
								   , const wchar_t * ) const
{
    (void)meta;
    (void)msize;

    translate_state result;

    if ( dsize > 0 ) {

        std::unique_ptr< infitof::AveragerData > avgr( new infitof::AveragerData );

        if ( infitof::serializer::deserialize( *avgr, data, dsize ) ) {

            if ( avgr->avgrType == infitof::Averager_AP240 ) {

                result = ap240translator::translate( xms, *avgr, idData, spectrometer );

            } else if ( avgr->avgrType == infitof::Averager_U5303A ) {

                result = u5303a_translator::translate( xms, *avgr, idData, spectrometer );
            }

            if ( avgr->protocolId == 0 )
                mslocker_->clear();

            adcontrols::segment_wrapper<> spectra( xms );
            if ( spectra.size() <= avgr->protocolId ) {
                ADDEBUG() << "## subscript out of range: spectra size: " << spectra.size() << ", #seg " << xms.numSegments() << ", proto:" << avgr->protocolId;
                return result; // subscript out of range
            }

            //auto& fms = spectra[ avgr->protocolId ];

            bool found( false );

            if ( result != adcontrols::translate_error ) {

#if 0 // real time lock mass trial
                if ( !avgr->protocol_.formulae_.empty() && avgr->protocol_.reference_ ) {

                    typedef adportable::waveform_peakfinder::peakinfo peakinfo;

                    auto mrange = std::make_pair( fms.getMass(0), fms.getMass( fms.size() - 1 ) );
                    double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( avgr->protocol_.formulae_ );

                    // ADDEBUG() << "lockmass: " << avgr->protocol_.formulae_ << " range(" << mrange.first << "," << mrange.second << ")";

                    if ( mrange.first < exactMass && exactMass < mrange.second ) {

                        auto first = std::lower_bound( fms.getMassArray(), fms.getMassArray() + fms.size(), exactMass - 1.5 );
                        auto last = std::lower_bound( fms.getMassArray(), fms.getMassArray() + fms.size(), exactMass + 1.5 );
                        size_t beg = std::distance( fms.getMassArray(), first );
                        size_t end = std::distance( fms.getMassArray(), last );

                        adportable::waveform_peakfinder finder( [this] ( size_t, int& npeakw ) { npeakw = int(npeaksamples_); return peakwidth_; } );
                        std::vector< peakinfo > results;

                        if ( finder( [fms] ( size_t idx ) { return fms.getTime( idx ); }, fms.getIntensityArray(), beg, end, results ) ) {

                            auto lit = std::lower_bound( results.begin(), results.end(), exactMass - 0.1
                                                         , [fms]( const peakinfo& a, double m ){ return fms.compute_mass( a.centreX ) < m; });
                            if ( lit != results.end() ) {
                                auto hit = std::lower_bound( lit, results.end(), exactMass + 0.1
                                                             , [fms]( const peakinfo& a, double m ){ return fms.compute_mass( a.centreX ) < m; });

                                auto it = std::max_element( lit, hit, []( const peakinfo& a, const peakinfo& b ){
                                        return a.height < b.height;
                                    });

                                if ( it != results.end() ) {
                                    found = true;

                                    double matchedMass = fms.compute_mass( it->centreX );
                                    *mslocker_ << adcontrols::lockmass::reference( avgr->protocol_.formulae_, exactMass, matchedMass, it->centreX );

                                    //adportable::waveform_peakfinder::parabola para;
                                    //double matchedMass2(0);
                                    //if ( finder.fit( [fms] ( size_t idx ) { return fms.getTime( idx ); }, fms.getIntensityArray(), ( *it ).spos, ( *it ).tpos, ( *it ).epos, para ) )
                                    //    matchedMass2 = fms.compute_mass( para.centreX );

                                    if ( !logfile_.empty() ) {

                                        std::ofstream of( logfile_.c_str(), std::ios_base::out | std::ios_base::app );

                                        // formula,tof(s),m/z,error(Da)
                                        of << boost::format( "\"%s\",%.14e,%.14f,%14e,%.7e,%.7e,\t" )
                                            % avgr->protocol_.formulae_
                                            % it->centreX
                                            % matchedMass
                                            % ( matchedMass - exactMass )
                                            % it->height
                                            % ( it->xright - it->xleft )
                                            ;
                                        if ( result == adcontrols::translate_complete )
                                            of << std::endl;
                                    }
                                }
                            }
                        }
                    }

                    if ( !found && !logfile_.empty() ) {
                        std::ofstream of( logfile_.c_str(), std::ios_base::out | std::ios_base::app );
                        of << boost::format( "%s NOT-FOUND,0,0,0,\t0,0,0,\t" ) % avgr->protocol_.formulae_;
                        if ( result == adcontrols::translate_complete )
                            of << std::endl;
                    }
                }
#endif
            }

            if ( found && result == adcontrols::translate_complete ) {
                if ( *mslocker_ && mslocker_->fit() ) {
                    // if ( ( *mslocker_ )( xms ) ) {
                    // ADDEBUG() << "lock mass applied.";
                    // }
                }
            }
            return result;
        }
    }
    return adcontrols::translate_error;
}

translate_state
InfiTofDataInterpreter::translate( adcontrols::TraceAccessor& accessor
                                   , const char * data, size_t dsize
                                   , const char * meta, size_t msize, unsigned long events ) const
{
    (void)meta;
    (void)msize;

	if ( dsize > 0 ) {

		std::vector< infitof::SpectrumProcessedData > vec;
		infitof::serializer::deserialize( vec, data, dsize );
		for ( const infitof::SpectrumProcessedData& d: vec ) {
			adcontrols::seconds_t sec( double( d.uptime ) * 1.0e-6 );
			accessor.push_back( d.fcn, d.npos, sec, d.tic, events );
		}
        return adcontrols::translate_complete;
	}
    return adcontrols::translate_error;
}

// make device_data to text
bool
InfiTofDataInterpreter::make_device_text( std::vector< std::pair< std::string, std::string > >& textv
                                          , const adcontrols::MSProperty& prop ) const
{
    textv.clear();

    try {
        // this was encoded by qtplatz/contrib/agilent/libs/acqrscontrols/u5303a/waveform.cpp

        acqrscontrols::u5303a::device_data d;
        if ( adportable::binary::deserialize<>()( d, prop.deviceData(), prop.deviceDataSize() ) ) {
            textv.emplace_back( "Identifier",      d.ident_.Identifier() );
            textv.emplace_back( "Revision",        d.ident_.Revision() );
            textv.emplace_back( "Vendor",          d.ident_.Vendor() );
            textv.emplace_back( "Description",     d.ident_.Description() );
            textv.emplace_back( "InstrumentModel", d.ident_.InstrumentModel() );
            textv.emplace_back( "FirmwareRevision",d.ident_.FirmwareRevision() );
            textv.emplace_back( "SerialNumber",    d.ident_.SerialNumber() );
            textv.emplace_back( "Options",         d.ident_.Options() );
            textv.emplace_back( "IOVersion",       d.ident_.IOVersion() );
            textv.emplace_back( "NbrADCBits",      ( boost::format("%1%") % d.ident_.NbrADCBits() ).str() );
            return true;
        }
    } catch ( ... ) {
    }
    return false;
}

// static
double
InfiTofDataInterpreter::compute_mass( double time, int mode, const adcontrols::MSCalibration& calib, const multumcontrols::ScanLaw& law )
{
    using namespace adcontrols;
    using namespace adcontrols::metric;

    if ( calib.algorithm() == MSCalibration::MULTITURN_NORMALIZED ) {
        double t0 = 0;
        if ( calib.t0_coeffs().empty() )
            return calib.compute_mass( scale_to( calib.time_prefix(), time - law.getTime(0, mode) ) / law.fLength( mode ) );

        double mass = law.getMass( time, mode );
		for ( int i = 0; i < 2; ++i ) {
            t0 = metric::scale_to_base( calib.compute( calib.t0_coeffs(), std::sqrt( mass ) ), calib.time_prefix() );
			double T  = scale_to( calib.time_prefix(), time - t0 );
			mass = calib.compute_mass( T / law.fLength( mode ) );
		}
        return mass;
    } else {
        double T = metric::scale_to( calib.time_prefix(), time );
        if ( ! calib.t0_coeffs().empty() )
            T -= calib.t0_coeffs()[0];
        return calib.compute_mass( T );
    }
    return 0;

}

void
InfiTofDataInterpreter::set_logfile( const char * logfile )
{
    logfile_ = logfile;
    std::ofstream of( logfile_.c_str(), std::ios_base::out | std::ios_base::app );
    of << adportable::date_string::logformat( std::chrono::system_clock::now(), true ) << std::endl;
}

void
InfiTofDataInterpreter::setProcessMethod( const adcontrols::ProcessMethod& pm )
{
    if ( ( pm_ = std::make_shared< adcontrols::ProcessMethod >( pm ) ) ) {
        if ( auto centroid = pm_->find< adcontrols::CentroidMethod >() ) {
            multumcontrols::infitof::ScanLaw law;

            double width = centroid->rsTofInDa();
            double mass = centroid->rsTofAtMz();
            peakwidth_ = law.getTime( mass + width, 0 ) - law.getTime( mass, 0 );
            npeaksamples_ = uint32_t( peakwidth_ / 0.5e-9 + 0.5 ) | 1; // odd
            ADDEBUG() << "process method: width=" << width << " @ m/z=" << mass << " --> width=" << peakwidth_ << " w=" << npeaksamples_;
        }
    }
}

///////////////////////
bool
InfiTofDataInterpreter::compile_header( adcontrols::MassSpectrum& ms, std::ifstream& in ) const
{
    return textfile::compile_header( ms, in );
}

bool
InfiTofDataInterpreter::has_lockmass() const
{
    return mslocker_ && !mslocker_->empty();
}

bool
InfiTofDataInterpreter::lockmass( adcontrols::lockmass::mslock& lkms ) const
{
    if ( mslocker_ && !mslocker_->empty() ) {
        lkms = *mslocker_;
        return true;
    }
    return false;
}
