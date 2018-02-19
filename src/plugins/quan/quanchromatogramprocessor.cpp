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

#include "quanchromatogramprocessor.hpp"
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
#include <adcontrols/waveform_filter.hpp>
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
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <algorithm>

using namespace quan;

// static
std::wstring
QuanChromatogramProcessor::make_title( const wchar_t * dataSource, const std::string& formula, int fcn, double error, const wchar_t * trailer )
{
    boost::filesystem::path path( dataSource );

    std::wstring title = ( boost::wformat( L"%s, %s#%d %.4f" ) % path.stem().wstring() % adportable::utf::to_wstring( formula ) % fcn % error ).str();

    title += trailer;  //L" (1st phase)";

    return title;
}

// static
std::wstring
QuanChromatogramProcessor::make_title( const wchar_t * dataSource, const QuanCandidate& candidate, const wchar_t * trailer )
{
    boost::filesystem::path path( dataSource );
    std::wstring title = ( boost::wformat( L"%s, %s" ) % path.stem().wstring() % adportable::utf::to_wstring( candidate.formula() ) ).str();
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
                mslock_ = std::make_shared< adcontrols::lockmass::mslock >();

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
QuanChromatogramProcessor::process1st( int64_t rowid
                                       , std::shared_ptr< adcontrols::MassSpectrum > ms
                                       , QuanSampleProcessor& sampleprocessor )
{
    correct_baseline( *ms );

    if ( mslockm_ && mslock_ ) {
        if ( auto raw = sampleprocessor.getLCMSDataset() ) {
            if ( raw->dataformat_version() <= 2 ) {
                bool locked = false;
                adcontrols::lockmass::mslock lkms;
                if ( raw->mslocker( lkms ) )
                    locked = lkms( *ms, true );
                if ( !locked )
                    doMSLock( *ms );
            }
        }
    }
    
    spectra_[ rowid ].profile = ms; // keep processed profile spectrum for second phase

    std::shared_ptr< adcontrols::MSPeakInfo > pkInfo;
    std::shared_ptr< adcontrols::MassSpectrum > centroid;
    std::shared_ptr< adcontrols::MassSpectrum > filtered;

    if ( doCentroid( *ms, *procm_, pkInfo, centroid, filtered ) ) {
        spectra_[ rowid ].centroid = centroid;
        spectra_[ rowid ].filtered = filtered;
        spectra_[ rowid ].mspkinfo = pkInfo;
    }

}

void
QuanChromatogramProcessor::find_parallel_chromatograms( std::vector< std::shared_ptr< QuanChromatograms > >& vec
                                                        , const std::vector< std::shared_ptr< QuanTarget > >& targets
                                                        , const std::string& reader_objtext
                                                        , double mspeak_width  // 0.002
                                                        , double tolerance )   // 0.010
                                                        
{
    for ( auto& t: targets ) {
        std::vector < QuanTarget::target_value > developped_target_values;
        t->compute_candidate_masses( mspeak_width, tolerance, developped_target_values );
        vec.emplace_back( std::make_shared< QuanChromatograms >( t->formula(), developped_target_values, reader_objtext ) );
    }

    for ( auto& sp : spectra_ ) {
        for ( auto& candidate: vec )
            candidate->append_to_chromatogram( sp.first, sp.second.profile, reader_objtext );
    }

    auto pCompounds = procm_->find< adcontrols::QuanCompounds >();

    for ( auto& qchro: vec ) {
        qchro->process_chromatograms( procm_ );
        if ( pCompounds )
            qchro->identify( pCompounds, procm_ );
        //if ( qchro->identify( pCompounds, procm_ ) )
        ; //qchro->refactor(); <- remove if not identified
    }
}

void
QuanChromatogramProcessor::save_candidate_chromatograms( std::shared_ptr< QuanDataWriter > writer
                                                         , const wchar_t * dataSource
                                                         , std::shared_ptr< const QuanChromatograms > chromatograms
                                                         , const wchar_t * title_trailer )
{
    for ( auto & chro : *chromatograms ) {
                
        std::string formula = chro->formula();
        std::wstring title = make_title( dataSource, formula, chro->fcn(), chro->matchedMass() - chro->exactMass(), title_trailer );
                
        if ( chro ) {

            if ( adfs::file file = writer->write( *chro->chromatogram(), title ) ) {

                chro->setDataGuid( file.name() ); // dataGuid for Chromatogarm on adfs
                
                if ( auto pkinfo = chro->peakResult() ) {

                    auto afile = writer->attach< adcontrols::PeakResult >( file, *pkinfo, pkinfo->dataClass() );
                    writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );

                }
            }
        }

    }
}

std::wstring
QuanChromatogramProcessor::save_spectrum( std::shared_ptr< QuanDataWriter > writer
                                          , const QuanCandidate& candidate
                                          , const std::wstring& title )
{
    std::wstring dataGuid;
        
    if ( auto profile = candidate.profile() ) {

        writer->remove( title, L"/Processed/Spectra" );

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
        }
    }
    return dataGuid;
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
                adcontrols::waveform_filter::fft4c::lowpass_filter( ms, pCentroidMethod->cutoffFreqHz() );
            }

            filtered->addDescription( adcontrols::description( L"process", dataproc::Constants::F_DFT_FILTERD ) );
            return QuanSampleProcessor::doCentroid( *pkInfo, *centroid, *filtered, *pCentroidMethod );

        } else {
            return QuanSampleProcessor::doCentroid( *pkInfo, *centroid, profile, *pCentroidMethod );
        }
    }
    return false;
}

void
QuanChromatogramProcessor::doit( QuanSampleProcessor& processor
                                 , adcontrols::QuanSample& sample
                                 , std::shared_ptr< QuanDataWriter > writer
                                 , const std::string& reader_objtext
                                 , std::shared_ptr< adwidgets::Progress > progress )
{
    uint32_t debug_level = 0;
    bool save_on_datasource = false;

    if ( auto qm = procm_->find< adcontrols::QuanMethod >() ) {
        debug_level = qm->debug_level();
        save_on_datasource = qm->save_on_datasource();
    }

    auto reader_name = reader_objtext.find( "histogram" ) != std::string::npos ? "histogram"
        : reader_objtext.find( "tdcdoc.waveform" ) != std::string::npos ? "waveform"
        : reader_objtext;

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

        double mass_width = 0.005; // 5mDa default
        if ( auto targeting_method = procm_->find< adcontrols::TargetingMethod >() ) {
            mass_width = targeting_method->tolerance( targeting_method->toleranceMethod() );
        }

        // extract candidate chromatograms for a target
        // find all peaks, and retention-time identification
        double tolerance = 0; // tolerance = 0 ==> single chromatogram per target (no parallel)
        std::vector< std::shared_ptr< QuanChromatograms > > qcrms_v1; // array of enum of chromatogram
        find_parallel_chromatograms( qcrms_v1, targets, reader_objtext, mass_width, tolerance );  

        if ( debug_level >= 4 ) {
            std::wstring trailer = L"(1st phase)," + adportable::utf::to_wstring( reader_name );
            for ( auto& qcrms : qcrms_v1 )
                save_candidate_chromatograms( writer, sample.dataSource(), qcrms, trailer.c_str() );
        }

        for ( auto& qcrms: qcrms_v1 ) {
            qcrms->refine_chromatograms( reader_objtext, candidates, [=]( int64_t rowid ){
                    auto it = spectra_.find( rowid );
                    return ( it != spectra_.end() ) ? it->second : QuanChromatograms::spectra_type();
                });
        }

        if ( debug_level >= 2 ) {
            for ( auto& qcrms : qcrms_v1 ) {
                std::wstring trailer = L"(2nd phase)," + adportable::utf::to_wstring( reader_name );
                save_candidate_chromatograms( writer, sample.dataSource(), qcrms, trailer.c_str() );
            }
        }

    } while ( 0 );

    // ---------- run quantitative analysis process ----------------
    // process based on found mass -- QuanChromatograms now point single chromatogram
    std::vector< std::shared_ptr< QuanChromatograms > > qcrms_v;
    for ( auto& candidate : candidates ) {
        qcrms_v.push_back( std::make_shared< QuanChromatograms >( candidate.formula(), candidate ) );
    }

    // re-generate chromatograms
    for ( auto& sp : spectra_ ) {
        for ( auto& candidate : qcrms_v )
            candidate->append_to_chromatogram( sp.first, sp.second.profile, reader_objtext );
    }

    std::pair< double, double > time_range =
        std::make_pair( spectra_.begin()->second.profile->getMSProperty().timeSinceInjection()
                      , spectra_.rbegin()->second.profile->getMSProperty().timeSinceInjection() );

    for ( auto& qchro : qcrms_v ) {
        std::for_each( qchro->begin(), qchro->end(), [=] ( std::shared_ptr<QuanChromatogram> c ) {
                c->chromatogram()->minimumTime( time_range.first );
                c->chromatogram()->maximumTime( time_range.second );
            });
    }

    auto pCompounds = procm_->find< adcontrols::QuanCompounds >();
    for ( auto& qchro : qcrms_v ) {

        qchro->process_chromatograms( procm_ );
        if ( pCompounds )
            qchro->identify( pCompounds, procm_ ); // retention time id

        qchro->finalize( [=] ( uint32_t pos ) {    // m/z id (if not identified yet)
                auto it = spectra_.find( pos );
                if ( it != spectra_.end() ) return it->second; else return QuanChromatograms::spectra_type(); } );
    }

    // save reference spectra
    for ( auto& qchro : qcrms_v ) {
        if ( auto candidate = qchro->quanCandidate() ) {

            std::wstring title = make_title( sample.dataSource(), *candidate );
            std::wstring dataGuid = save_spectrum( writer, *candidate, title );
            if ( ! dataGuid.empty() ) {
                std::for_each( qchro->begin(), qchro->end(), [=] ( std::shared_ptr<QuanChromatogram> c ) {
                        c->setReferenceDataGuid( dataGuid, candidate->idx(), candidate->fcn() );
                    } );
            }
        }
    }

    // identify all
	if ( auto pCompounds = procm_->find< adcontrols::QuanCompounds >() ) {
        for ( auto& qchro: qcrms_v ) {
            std::wstring trailer = L"(final)," + adportable::utf::to_wstring( reader_name );
            save_candidate_chromatograms( writer, sample.dataSource(), qchro, trailer.c_str() );

            std::for_each( qchro->begin(), qchro->end(), [&] ( std::shared_ptr<QuanChromatogram> c ) {

                    std::string formula = c->formula();
                    auto itCmpd = std::find_if( pCompounds->begin(), pCompounds->end()
                                                , [c] ( const adcontrols::QuanCompound& cmpd ) { return cmpd.formula() == c->formula(); } );
                    if ( itCmpd != pCompounds->end() ) {

                        auto& peaks = c->peakResult()->peaks();
                        auto pk = std::find_if( peaks.begin(), peaks.end()
                                                , [c] ( const adcontrols::Peak& p ) {
                                                    return std::string( p.formula() ) == c->formula(); } );

                        if ( pk != peaks.end() ) {

                            adcontrols::QuanResponse resp;

                            resp.dataGuid_ = c->dataGuid();
                            resp.mass_ = c->matchedMass();
                            resp.formula( c->formula().c_str() );
                            resp.uuid_cmpd( itCmpd->uuid() );
                            resp.uuid_cmpd_table( pCompounds->uuid() );
                            resp.idx_ = pk->peakId();
                            resp.fcn_ = c->fcn();
                            resp.intensity_ = pk->peakArea();
                            resp.amounts_ = 0;
                            resp.tR_ = pk->peakTime(); // double( adcontrols::timeutil::toMinutes( pk->peakTime() ) );

                            sample << resp;
                        }
                    }
                });
        }
    }

    for ( auto& qchro : qcrms_v ) {
        for ( auto & c : *qchro )
            writer->insert_table( c->dataGuid(), c->referenceDataGuids() );
    }
    
    if ( save_on_datasource ) {
        if ( auto source = std::make_shared< QuanDataWriter >( sample.dataSource() ) ) {
            if ( source->open() ) {

                // save reference spectra
                for ( auto& qchro : qcrms_v ) {

                    if ( auto candidate = qchro->quanCandidate() )
                        save_spectrum( source, *candidate, make_title( sample.dataSource(), *candidate ) );

                    for ( auto& chro: *qchro ) {
                        
                        std::wstring title = make_title( sample.dataSource(), chro->formula(), chro->fcn(), chro->matchedMass() - chro->exactMass(), L"(final)" );
                        writer->remove( title, L"/Processed/Chromatograms" );
                        
                        if ( adfs::file file = writer->write( *chro->chromatogram(), title ) ) {
                            if ( auto pkinfo = chro->peakResult() ) {
                                auto afile = writer->attach< adcontrols::PeakResult >( file, *pkinfo, pkinfo->dataClass() );
                                writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );
                            }
                        }
                    }
                }
            }
        }
    }
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
                auto cseg = std::make_shared< adcontrols::MassSpectrum >();
                peak_detector( profile.getSegment( fcn ) );
                peak_detector.getCentroidSpectrum( *cseg );
                *centroid << std::move( cseg ); 
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
