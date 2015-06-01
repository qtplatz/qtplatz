/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "quanchromatogramsprocessor.hpp"
#include "quancandidate.hpp"
#include "quanchromatograms.hpp"
#include "quanchromatogram.hpp"
#include "quandatawriter.hpp"
#include "quanprocessor.hpp"
#include "quansampleprocessor.hpp"
#include "quantarget.hpp"
#include "quandocument.hpp"
#include "../plugins/dataproc/dataprocconstants.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanresponse.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/debug.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/cpio.hpp>
#include <adlog/logger.hpp>
#include <adutils/cpio.hpp>
#include <adwidgets/progresswnd.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <algorithm>

using namespace quan;

// static
std::wstring
QuanChromatogramProcessor::make_title( const wchar_t * dataSource, const std::string& formula, double error, const wchar_t * trailer )
{

    boost::filesystem::path path( dataSource );

    std::wstring title = ( boost::wformat( L"%s, %s %.4f" ) % path.stem().wstring() % adportable::utf::to_wstring( formula ) % error ).str();

    title += trailer;  //L" (1st phase)";

    return title;
}


QuanChromatogramProcessor::QuanChromatogramProcessor( std::shared_ptr< const adcontrols::ProcessMethod > pm )
    : procm_( pm )
{
    
    if ( auto pCompounds = procm_->find< adcontrols::QuanCompounds >() ) {
        if ( auto lkm = procm_->find< adcontrols::MSLockMethod >() ) {

            if ( lkm->enabled() ) {
                
                mslockm_ = std::make_shared< adcontrols::MSLockMethod >( *lkm );
                mslock_ = std::make_shared< adcontrols::lockmass >();

            }
        }
    }
    
    if ( auto compounds = procm_->find< adcontrols::QuanCompounds >() ) {
        
        adcontrols::ChemicalFormula parser;
        
        for ( auto& comp : *compounds ) {
            
            std::string formula( comp.formula() );
            double exactMass = parser.getMonoIsotopicMass( formula );
            
            if ( ! formula.empty() ) {

                if ( comp.isLKMSRef() )
                    references_.push_back( exactMass );
                
            }
        }

    }
}

void
QuanChromatogramProcessor::process1st( size_t pos, std::shared_ptr< adcontrols::MassSpectrum > ms, QuanSampleProcessor& sampleprocessor )
{
    correct_baseline( *ms );

    if ( mslockm_ && mslock_ ) {
        bool locked = false;
        if ( auto raw = sampleprocessor.getLCMSDataset() ) {
            adcontrols::lockmass lkms;
            if ( raw->mslocker( lkms ) )
                locked = lkms( *ms, true );
        }
        if ( !locked )
            doMSLock( *ms );
    }
    
    spectra_[ pos ].profile = ms; // keep processed profile spectrum for second phase

    std::shared_ptr< adcontrols::MSPeakInfo > pkInfo;
    std::shared_ptr< adcontrols::MassSpectrum > centroid;
    std::shared_ptr< adcontrols::MassSpectrum > filtered;

    if ( doCentroid( *ms, *procm_, pkInfo, centroid, filtered ) ) {
        spectra_[ pos ].centroid = centroid;
        spectra_[ pos ].filtered = filtered;
        spectra_[ pos ].mspkinfo = pkInfo;
    }

}

void
QuanChromatogramProcessor::find_parallel_chromatograms( std::vector< std::shared_ptr< QuanChromatograms > >& vec
                                                        , const std::vector< std::shared_ptr< QuanTarget > >& targets )
{
    for ( auto& t: targets ) {

        // a target := a molecule|compound <- several trial chromatograms (0.002Da step)
        
        std::vector < QuanTarget::target_value > developped_target_values;
        t->compute_candidate_masses( 0.002, 0.010, developped_target_values );
        
        vec.push_back( std::make_shared< QuanChromatograms >( t->formula(), developped_target_values ) );
    }

    for ( auto& sp : spectra_ ) {
        for ( auto& candidate: vec )
            candidate->append_to_chromatogram( sp.first, sp.second.profile );
    }

    auto pCompounds = procm_->find< adcontrols::QuanCompounds >();

    for ( auto& candidate: vec ) {
        candidate->process_chromatograms( procm_ );
        if ( pCompounds )
            candidate->identify( pCompounds, procm_ );
    }
}

void
QuanChromatogramProcessor::find_candidates( std::vector< std::shared_ptr< QuanChromatograms > >& vec, std::vector<QuanCandidate>& refined )
{
    for ( auto& qcrms: vec ) {

        // candidate has the parallel processed chromatograms against single target mass

        qcrms->refine_chromatograms( refined, [=](uint32_t pos){
                auto it = spectra_.find( pos );
                if ( it != spectra_.end() )
                    return it->second;
                else
                    return QuanChromatograms::spectra_type();
            });

    }
}

void
QuanChromatogramProcessor::save_candidate_chromatograms( std::shared_ptr< QuanDataWriter > writer
                                                         , const wchar_t * dataSource
                                                         , std::shared_ptr< const QuanChromatograms > chromatograms
                                                         , const wchar_t * title_trailer )
{
    for ( auto & chro : *chromatograms ) {
                
        std::string formula = chro->formula_;
        std::wstring title = make_title( dataSource, formula, chro->matchedMass_ - chro->exactMass_, title_trailer );
                
        if ( chro ) {
            
            if ( adfs::file file = writer->write( *chro->chromatogram_, title ) ) {

                chro->dataGuid_ = file.name(); // dataGuid for Chromatogarm on adfs
                
                if ( auto pkinfo = chro->peakinfo_ ) {
                    
                    if ( !chro->resp_ )
                        chro->resp_ = std::make_shared< adcontrols::QuanResponse >();

                    chro->resp_->dataGuid_ = chro->dataGuid_;
                            
                    auto afile = writer->attach< adcontrols::PeakResult >( file, *pkinfo, pkinfo->dataClass() );
                    writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );
                }
            }
        }

    }
}

bool
QuanChromatogramProcessor::save_candidate_spectra( std::shared_ptr< QuanDataWriter > writer
                                                   , std::wstring& dataGuid
                                                   , const wchar_t * dataSource
                                                   , const QuanCandidate& candidate
                                                   , const wchar_t * title_trailer )
{
    boost::filesystem::path path( dataSource );
    
    std::wstring title = ( boost::wformat( L"%s, %s" ) % path.stem().wstring() % adportable::utf::to_wstring( candidate.formula() ) ).str();
    title += title_trailer;

    if ( auto profile = candidate.profile() ) {

        if ( adfs::file file = writer->write( *profile, title ) ) {
            if ( auto filtered = candidate.filtered() ) {
                writer->attach< adcontrols::MassSpectrum >( file, *filtered, dataproc::Constants::F_DFT_FILTERD );
            }
            if ( auto centroid = candidate.centroid() ) {
                auto afile = writer->attach< adcontrols::MassSpectrum >( file, *centroid, dataproc::Constants::F_CENTROID_SPECTRUM );
                writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );
            }
            if ( auto pkinfo = candidate.mspkinfo() ) {
                writer->attach< adcontrols::MSPeakInfo >( file, *pkinfo, dataproc::Constants::F_MSPEAK_INFO );
            }
            dataGuid = file.name();
            return true;
        }
    }
    return false;
}
        
//static
bool
QuanChromatogramProcessor::doCentroid( const adcontrols::MassSpectrum& profile
                        , const adcontrols::ProcessMethod& pm
                        , std::shared_ptr< adcontrols::MSPeakInfo >& pkInfo
                        , std::shared_ptr< adcontrols::MassSpectrum >& centroid
                        , std::shared_ptr< adcontrols::MassSpectrum >& filtered )
{
    pkInfo = std::make_shared< adcontrols::MSPeakInfo >();
    centroid = std::make_shared< adcontrols::MassSpectrum >();
    filtered.reset();

    if ( auto pCentroidMethod = pm.find< adcontrols::CentroidMethod >() ) {

        if ( pCentroidMethod->noiseFilterMethod() == adcontrols::CentroidMethod::eDFTLowPassFilter ) {

            auto filtered = std::make_shared< adcontrols::MassSpectrum >();
            filtered->clone( profile, true );

            for ( auto& ms : adcontrols::segment_wrapper<>( *filtered ) ) {
                adcontrols::waveform::fft::lowpass_filter( ms, pCentroidMethod->cutoffFreqHz() );
            }

            filtered->addDescription( adcontrols::description( L"process", dataproc::Constants::F_DFT_FILTERD ) );
            return QuanSampleProcessor::doCentroid( *pkInfo, *centroid, *filtered, *pCentroidMethod );

        } else {
            return QuanSampleProcessor::doCentroid( *pkInfo, *centroid, profile, *pCentroidMethod );
        }
    }
    return false;
}

#if 0
bool
QuanChromatogramProcessor::identify_cpeak( adcontrols::QuanResponse& resp
                                           , const std::string& formula
                                           , std::shared_ptr< adcontrols::Chromatogram > chro
                                           , std::shared_ptr< adcontrols::PeakResult > pkResult )
{
	if ( auto pCompounds = procm_->find< adcontrols::QuanCompounds >() ) {

		auto itCompound = std::find_if( pCompounds->begin(), pCompounds->end(), [formula] ( const adcontrols::QuanCompound& c ) {
			return c.formula() == formula;
		} );

		// Identify chromatographic peak
		if ( pkResult ) {
			auto& pks = pkResult->peaks();
			if ( itCompound != pCompounds->end() && pks.size() > 0 ) {

				auto pk = std::max_element( pks.begin(), pks.end()
					, [] ( const adcontrols::Peak& a, const adcontrols::Peak& b ) { return a.peakHeight() < b.peakHeight(); } );

				// assign peak name
				pk->name( adportable::utf::to_wstring( formula ) );

				// set response 
				resp.formula( formula.c_str() );
				resp.uuid_cmpd( itCompound->uuid() );
				resp.uuid_cmpd_table( pCompounds->uuid() );
				resp.formula( itCompound->formula() );
				resp.idx_ = pk->peakId();
				resp.fcn_ = chro->fcn();
				//resp.mass_ = mass;
				resp.intensity_ = pk->peakArea();
				resp.amounts_ = 0;
				resp.tR_ = double( adcontrols::timeutil::toMinutes( pk->peakTime() ) );
				//resp.dataGuid_ = dataGuid;
				return true;
			}
		}
	}
    return false;
}
#endif

void
QuanChromatogramProcessor::doit( QuanSampleProcessor& processor, adcontrols::QuanSample& sample
                               , std::shared_ptr< QuanDataWriter > writer, std::shared_ptr< adwidgets::Progress > progress )
{
    std::vector< QuanCandidate > candidates;

    do { // first phase
        std::vector< std::shared_ptr< QuanTarget > > targets;

        if ( auto compounds = procm_->find< adcontrols::QuanCompounds >() ) {

            for ( auto& comp : *compounds ) {

                std::string formula( comp.formula() );
                if ( !formula.empty() ) {
                    targets.push_back( std::make_shared< QuanTarget >( formula ) );
                }
            }
        }

        std::vector< std::shared_ptr< QuanChromatograms > > qcrms_v1;
        find_parallel_chromatograms( qcrms_v1, targets );
#if 0
        for ( auto& qcrms : qcrms_v1 )
            save_candidate_chromatograms( writer, sample.dataSource(), qcrms, L"(1st phase)" );
#endif
        find_candidates( qcrms_v1, candidates );

        for ( auto& qcrms : qcrms_v1 )
            save_candidate_chromatograms( writer, sample.dataSource(), qcrms, L"(2nd phase)" );

    } while ( 0 );

    // ---------- run quantitative analysis process ----------------
    // process based on found mass
    std::vector< std::shared_ptr< QuanChromatograms > > qcrms_v;
    for ( auto& candidate : candidates ) {
        qcrms_v.push_back( std::make_shared< QuanChromatograms >( candidate.formula(), candidate ) );
    }
    
    for ( auto& sp : spectra_ ) {
        for ( auto& candidate : qcrms_v )
            candidate->append_to_chromatogram( sp.first, sp.second.profile );
    }

    auto pCompounds = procm_->find< adcontrols::QuanCompounds >();
    for ( auto& qchro : qcrms_v ) {
        qchro->process_chromatograms( procm_ );
        if ( pCompounds )
            qchro->identify( pCompounds, procm_ );
        qchro->finalize( [=] ( uint32_t pos ) { auto it = spectra_.find( pos ); if ( it != spectra_.end() ) return it->second; else return QuanChromatograms::spectra_type(); } );
    }

    for ( auto& qchro : qcrms_v ) {
        if ( auto candidate = qchro->quanCandidate() ) {
            std::wstring dataGuid;
            if ( save_candidate_spectra( writer, dataGuid, sample.dataSource(), *candidate ) )
                qchro->setReferenceDataGuid( dataGuid );
        }
    }

    for ( auto& qchro : qcrms_v ) {
        save_candidate_chromatograms( writer, sample.dataSource(), qchro, L"(final)" );
        // for ( auto & c : *qchro ) {
        //     c->dataGuid_
        // }
    }
    
#if 0
    // save_candidate_spectra( writer, sample, progress );
    std::for_each( chroms_->begin(), chroms_->end(), [=]( QuanChromatogram& c ){

            std::wstring title = make_title( sample.dataSource(), c.formula_, QuanChromatograms::_2nd );
            
            if ( auto profile = c.profile_ ) {

                if ( adfs::file file = writer->write( *profile, title ) ) {
                    
                    c.dataGuids_.push_back( std::make_tuple( file.name(), c.idxfcn_.first, c.idxfcn_.second ) );
                    
                    if ( auto filtered = c.filtered_ ) {
                        writer->attach< adcontrols::MassSpectrum >( file, *filtered, dataproc::Constants::F_DFT_FILTERD );
                    }
                    if ( auto centroid = c.centroid_ ) {
                        auto afile = writer->attach< adcontrols::MassSpectrum >( file, *centroid, dataproc::Constants::F_CENTROID_SPECTRUM );
                        writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );
                    }
                    if ( auto pkinfo = c.mspeaks_ ) {
                        writer->attach< adcontrols::MSPeakInfo >( file, *pkinfo, dataproc::Constants::F_MSPEAK_INFO );
                    }
                }
            }
        });

    for ( auto& sp : spectra_ ) {
        chroms_->process2nd( sp.first, *std::get< idProfile >( sp.second ) );
        progress && ( *progress )();        
    }
    
    process_chromatograms( processor, QuanChromatograms::_2nd );
    std::for_each( chroms_->begin(), chroms_->end(), [=]( QuanChromatogram& c ){
            // c.resp_ will override
            identify_cpeak( *c.resp_, c.formula_, c.cmgrs_[ QuanChromatograms::_2nd ], c.pkres_[ QuanChromatograms::_2nd ] );
            c.resp_->mass_ = c.matchedMass_;
        });

    // save chromatograms
    for ( auto phase: { QuanChromatograms::_1st, QuanChromatograms::_2nd } )
        save_candidate_chromatograms( writer, sample.dataSource(), phase, progress );

    // save reference dataGuids
    std::for_each( chroms_->begin(), chroms_->end(), [=]( QuanChromatogram& c ){
            for ( auto guid: c.dataGuid_ )
                writer->insert_table( guid, c.dataGuids_ );
        });
    
    std::for_each( chroms_->begin(), chroms_->end()
                   , [&] ( const QuanChromatogram& c ) { if ( c.resp_ ) sample << *c.resp_; } );

#endif
}

void
QuanChromatogramProcessor::correct_baseline( adcontrols::MassSpectrum& profile )
{
    if ( profile.isCentroid() )
        return;
    adcontrols::segment_wrapper<> segments( profile );

    for ( auto& ms: segments ) {
        double dbase(0), rms(0), tic(0);
        const double * data = ms.getIntensityArray();
        tic = adportable::spectrum_processor::tic( static_cast< unsigned int >( ms.size() ), data, dbase, rms );

        for ( size_t idx = 0; idx < ms.size(); ++idx )
            ms.setIntensity( idx, data[ idx ] - dbase );
    }
}

bool
QuanChromatogramProcessor::doMSLock( adcontrols::MassSpectrum& profile )
{
    //--------- centroid --------
    auto centroid = std::make_shared< adcontrols::MassSpectrum >();

    if ( auto m = procm_->find< adcontrols::CentroidMethod >() ) {

        adcontrols::CentroidProcess peak_detector;

        adcontrols::segment_wrapper<> segments( profile );
        
        centroid->clone( profile, false );

        if ( peak_detector( *m, profile ) ) {
            peak_detector.getCentroidSpectrum( *centroid );

            for ( auto fcn = 0; fcn < profile.numSegments(); ++fcn ) {
                adcontrols::MassSpectrum cseg;
                peak_detector( profile.getSegment( fcn ) );
                peak_detector.getCentroidSpectrum( cseg );
                centroid->addSegment( cseg );
            }

        }
    }
    
    //--------- lockmass --------
    // this does not find reference from segments attached to 'centroid'
    if ( centroid ) {
        
        adcontrols::MSFinder find( mslockm_->tolerance( mslockm_->toleranceMethod() ), mslockm_->algorithm(), mslockm_->toleranceMethod() );

        adcontrols::segment_wrapper<> segments( *centroid );
        
        if ( auto pCompounds = procm_->find< adcontrols::QuanCompounds >() ) {
            for ( auto& compound: *pCompounds ) {

                if ( compound.isLKMSRef() ) {
                    double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( compound.formula() );                    

                    for ( auto& fms: segments ) {
                        if ( fms.getMass( 0 ) < exactMass && exactMass < fms.getMass( fms.size() - 1 ) ) {
                            auto idx = find( fms, exactMass );
                            if ( idx != adcontrols::MSFinder::npos ) {
                                *mslock_ << adcontrols::lockmass::reference( compound.formula(), exactMass, fms.getMass( idx ), fms.getTime( idx ) );
                            }
                        }
                    }
                }
            }
        }
    }
    if ( mslock_ )
        mslock_->fit();
    ( *mslock_ )( profile, true );
    return true;
}

