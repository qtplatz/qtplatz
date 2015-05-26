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

#include "quansampleprocessor.hpp"
#include "quanprocessor.hpp"
#include "quanchromatograms.hpp"
#include "quandatawriter.hpp"
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
#include <algorithm>

namespace quan {

    class QuanChromatogramProcessor {
        std::shared_ptr< const adcontrols::ProcessMethod > procm_;
    public:
        QuanChromatogramProcessor( std::shared_ptr< const adcontrols::ProcessMethod > pm ) : procm_( pm )
                                                                                           , chroms_( std::make_shared< QuanChromatograms >( pm ) ) {
            
        }

        inline void process1st( size_t pos, std::shared_ptr< adcontrols::MassSpectrum > ms, QuanSampleProcessor& sampleprocessor ) {

            // baseline collection, lock mass, extract MS peak area for chromatogram sample point
            chroms_->process1st( pos, *ms, sampleprocessor );

            spectra_[ pos ] = ms; // keep processed profile spectrum for second phase
        }

        inline void process2nd( std::shared_ptr< adwidgets::Progress > progress ) {
            std::for_each( chroms_->begin(), chroms_->end(), []( QuanChromatograms::target_t& t ){
                    auto& range = std::get< QuanChromatograms::idMSWidth >( t );
                    std::get< QuanChromatograms::idChromatogram >( t ) = std::make_shared< adcontrols::Chromatogram >();
                });
            for ( auto& sp: spectra_ )
                chroms_->process2nd( sp.first, *sp.second );
        }
        
        inline void commit( QuanSampleProcessor& sampleprocessor ) {
            chroms_->commit( sampleprocessor );
        }

        inline std::wstring make_title( const adcontrols::QuanSample& sample, QuanChromatograms::const_iterator it, bool isFinal ) {

            std::string formula = std::get< QuanChromatograms::idFormula >( *it );

            boost::filesystem::path path( sample.dataSource() );
            if ( isFinal )
                return path.stem().wstring() + L", " + adportable::utf::to_wstring( formula );                
            else
                return path.stem().wstring() + L", (" + adportable::utf::to_wstring( formula ) + L")";

        }

        inline void write_candidate_chromatograms( std::shared_ptr< QuanDataWriter > writer
                                                   , adcontrols::QuanSample& sample
                                                   , bool is2nd
                                                   , std::shared_ptr< adwidgets::Progress > progress ) {
            
            for ( auto it = chroms_->begin(); it != chroms_->end(); ++it ) {
                
                std::string formula = std::get< QuanChromatograms::idFormula >( *it );
                
                std::wstring title = make_title( sample, it, is2nd );

                if ( auto chro = std::get< QuanChromatograms::idChromatogram >( *it ) ) {

                    if ( adfs::file file = writer->write( *chro, title ) ) {

                        if ( auto pkres = std::get< QuanChromatograms::idPeakResult >( *it ) ) {

                            double mass = std::get< QuanChromatograms::idMass >( *it );
                            auto resp = std::make_shared< adcontrols::QuanResponse >();

                            if ( find_target_peak( *resp, formula, mass, chro, pkres, file.name() ) ) {

                                std::get< QuanChromatograms::idQuanResponse >( *it ) = resp;
                                if ( is2nd )
                                    sample << *resp;
                            }
                            auto afile = writer->attach< adcontrols::PeakResult >( file, *pkres, pkres->dataClass() );
                            writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );
                        }
                    }
                }
                progress && (*progress)();                    
            }
        }
        
        static bool doCentroid( const adcontrols::MassSpectrum& profile
                                , const adcontrols::ProcessMethod& pm
                                , std::shared_ptr< adcontrols::MSPeakInfo >& pkInfo
                                , std::shared_ptr< adcontrols::MassSpectrum >& centroid
                                , std::shared_ptr< adcontrols::MassSpectrum >& filtered ) {

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

        inline void collect_candidate_spectra( std::shared_ptr< adwidgets::Progress > progress ) {

            for ( auto it = chroms_->begin(); it != chroms_->end(); ++it ) {

                if ( auto resp = std::get< QuanChromatograms::idQuanResponse >( *it ) ) {
                    if ( auto pkres = std::get< QuanChromatograms::idPeakResult >( *it ) ) {
                        auto pk = pkres->peaks().find_peakId( resp->idx_ );
                        if ( pk != pkres->peaks().end() ) {
                            auto spIt = std::lower_bound( spectra_.begin(), spectra_.end(), pk->peakTime()
                                                          , []( const std::pair< size_t, std::shared_ptr< adcontrols::MassSpectrum > >& s, double t ){
                                                              return s.second->getMSProperty().timeSinceInjection() < t;
                                                          });
                            if ( spIt != spectra_.end() ) {
                                std::get< QuanChromatograms::idSpectrum >( *it ) = spIt->second;

                                std::shared_ptr< adcontrols::MSPeakInfo > pkInfo;
                                std::shared_ptr< adcontrols::MassSpectrum > centroid;
                                std::shared_ptr< adcontrols::MassSpectrum > filtered;

                                if ( doCentroid( *spIt->second, *procm_, pkInfo, centroid, filtered ) ) {
                                    if ( filtered )
                                        std::get< QuanChromatograms::idSpectrum >( *it ) = filtered;
                                    std::get< QuanChromatograms::idCentroid >( *it ) = centroid;
                                    std::get< QuanChromatograms::idMSPeakInfo >( *it ) = pkInfo;
                                }
                            }
                        }
                    }
                }
            }
        }

        inline void find_candidate_mspeak( std::shared_ptr< adwidgets::Progress > progress ) {
           
            if ( auto tm = procm_->find< adcontrols::TargetingMethod >() ) {

                double tolerance = tm->tolerance( adcontrols::idToleranceDaltons );

                adcontrols::MSFinder find( tolerance, adcontrols::idFindLargest, adcontrols::idToleranceDaltons );
                
                for ( auto it = chroms_->begin(); it != chroms_->end(); ++it ) {
                    size_t fcn = 0;
                    double exactMass = std::get< QuanChromatograms::idMass >( *it );
                    if ( auto centroid = std::get < QuanChromatograms::idCentroid >( *it ) ) {
                        for ( auto& fms : adcontrols::segment_wrapper<>( *centroid ) ) {
                            size_t idx = find( fms, exactMass );
                            if ( idx != adcontrols::MSFinder::npos ) {
                                auto pkinf = adcontrols::segment_wrapper< adcontrols::MSPeakInfo >( *std::get< QuanChromatograms::idMSPeakInfo >( *it ) )[ fcn ];
                                if ( idx < pkinf.size() ) {
                                    auto pk = pkinf.begin() + idx;
                                    std::get< QuanChromatograms::idIdxFcn>( *it ) = std::make_pair( idx, fcn );
                                    std::get< QuanChromatograms::idMSWidth>( *it ) = std::make_pair( pk->centroid_left(), pk->centroid_right() );

                                    auto formula = std::get< QuanChromatograms::idFormula >( *it );
                                    auto exactMass = std::get< QuanChromatograms::idMass >( *it );
                                    using adcontrols::annotation;
                                    annotation anno( formula, pk->mass(), pk->height(), int( idx ), pk->height(), annotation::dataFormula );
                                    fms.get_annotations() << anno;
                                }
                            }
                            ++fcn;
                        }
                    }
                }
            }
        }
        
        
        inline bool find_target_peak( adcontrols::QuanResponse& resp
                                   , const std::string& formula
                                   , double mass
                                   , std::shared_ptr< adcontrols::Chromatogram > chro
                                   , std::shared_ptr< adcontrols::PeakResult > pkResult
                                   , const std::wstring& dataGuid )  {
            
            if ( auto pCompounds = procm_->find< adcontrols::QuanCompounds >() ) {
                
                auto itCompound = std::find_if( pCompounds->begin(), pCompounds->end(), [formula] ( const adcontrols::QuanCompound& c ) {
                        return c.formula() == formula;
                    } );
                
                // Identify chromatographic peak
                
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
                    resp.mass_ = mass;
                    resp.intensity_ = pk->peakArea();
                    resp.amounts_ = 0;
                    resp.tR_ = double( adcontrols::timeutil::toMinutes( pk->peakTime() ) );
                    resp.dataGuid_ = dataGuid;
                    
                    return true;
                }
            }
            return false;
        }

        inline void write_candidate_spectra( std::shared_ptr< QuanDataWriter > writer
                                             , adcontrols::QuanSample& sample
                                             , std::shared_ptr< adwidgets::Progress > progress ) {

            for ( auto it = chroms_->begin(); it != chroms_->end(); ++it ) {
                std::wstring title = make_title( sample, it, false );
                if ( auto profile = std::get< QuanChromatograms::idSpectrum >( *it ) ) {
                    if ( adfs::file file = writer->write( *profile, title ) ) {
                        if ( auto centroid = std::get< QuanChromatograms::idCentroid >( *it ) ) {
                            auto afile = writer->attach< adcontrols::MassSpectrum >( file, *centroid, dataproc::Constants::F_CENTROID_SPECTRUM );
                            writer->attach< adcontrols::ProcessMethod >( afile, *procm_, L"ProcessMethod" );
                            if ( auto pkinfo = std::get< QuanChromatograms::idMSPeakInfo >( *it ) ) {
                                writer->attach< adcontrols::MSPeakInfo >( file, *pkinfo, dataproc::Constants::F_MSPEAK_INFO );
                            }
                        }
                    }
                }
            }
        }
        
        std::shared_ptr< QuanChromatograms > chroms_;
        std::map< size_t, std::shared_ptr< adcontrols::MassSpectrum > > spectra_;
    };
    
}

using namespace quan;

QuanSampleProcessor::~QuanSampleProcessor()
{
}

QuanSampleProcessor::QuanSampleProcessor( QuanProcessor * processor
                                        , std::vector< adcontrols::QuanSample >& samples )
                                        : raw_( 0 )
                                        , samples_( samples )
                                        , procmethod_( processor->procmethod() )
                                        , cformula_( std::make_shared< adcontrols::ChemicalFormula >() )
                                        , processor_( processor->shared_from_this() )
                                        , progress_( adwidgets::ProgressWnd::instance()->addbar() )
                                        , progress_current_( 0 )
                                        , progress_total_( 0 )
{
    if ( !samples.empty() )
        path_ = samples[ 0 ].dataSource();
}

QuanProcessor *
QuanSampleProcessor::processor()
{
    return processor_.get();
}

void
QuanSampleProcessor::dryrun()
{
    nFcn_ = 0;
    nSpectra_ = 0;
    adcontrols::Chromatogram tic;
    if ( raw_ ) {
        nFcn_ = raw_->getFunctionCount();
        raw_->getTIC( 0, tic );
        nSpectra_ = tic.size();
    }

    for ( auto& sample : samples_ ) {
        switch ( sample.dataGeneration() ) {
        case adcontrols::QuanSample::GenerateSpectrum:
            if ( nFcn_ && sample.scan_range_first() == 0 && sample.scan_range_second() == static_cast< unsigned int>(-1) )
                progress_total_ += int( nSpectra_ / nFcn_ );
            progress_total_++;
            break;
        case adcontrols::QuanSample::ProcessRawSpectra:
            progress_total_ += int( nSpectra_ );
            break;
        case adcontrols::QuanSample::ASIS:
            progress_total_++;
            break;
        case adcontrols::QuanSample::GenerateChromatogram:
            if ( procmethod_ ) {
                if ( auto qm = procmethod_->find< adcontrols::QuanMethod >() ) {
                    progress_total_ = int( nSpectra_ ) * 2;
                }
                if ( auto pCompounds = procmethod_->find< adcontrols::QuanCompounds >() ) {
                    progress_total_ += int( pCompounds->size() );
                }
                progress_total_++;                
            }
            
            break;
        }
    }
    progress_current_ = 0;
    progress_->setRange( int( progress_current_ ), int( progress_total_ ) );
}

bool
QuanSampleProcessor::operator()( std::shared_ptr< QuanDataWriter > writer )
{
    open();

    dryrun();

    for ( auto& sample : samples_ ) {

        switch ( sample.dataGeneration() ) {

        case adcontrols::QuanSample::GenerateChromatogram:
            if ( raw_ ) {
                auto chromatogram_processor = std::make_shared< QuanChromatogramProcessor >( procmethod_ );
                auto ms = std::make_shared< adcontrols::MassSpectrum >();
                size_t pos = 0;
                while ( pos = read_raw_spectrum( pos, raw_, *ms ) ) {
                    chromatogram_processor->process1st( pos, ms, *this );
                    if ( (*progress_)() )
                        return false;
                    ms = std::make_shared< adcontrols::MassSpectrum >();
                }
                chromatogram_processor->commit( *this );

                chromatogram_processor->write_candidate_chromatograms( writer, sample, false, progress_ );
                chromatogram_processor->collect_candidate_spectra( progress_ );
                chromatogram_processor->find_candidate_mspeak( progress_ );
                chromatogram_processor->write_candidate_spectra( writer, sample, progress_ );
                chromatogram_processor->process2nd( progress_ );
                chromatogram_processor->commit( *this );
                chromatogram_processor->write_candidate_chromatograms( writer, sample, true, progress_ );

                writer->insert_table( sample ); // once per sample
#if ! defined _DEBUG
                // only for release build
                // save on original ('RUN_XXXX.adfs') file
                datafile_->saveContents( L"/Processed", *portfolio_ ); 
#endif
                (*progress_)();
            }
            break; // ignore for this version

        case adcontrols::QuanSample::GenerateSpectrum:
            if ( raw_ ) {
                adcontrols::MassSpectrum ms;
                if ( generate_spectrum( raw_, sample, ms ) ) {
                    adcontrols::segments_helper::normalize( ms ); // normalize to 10k average equivalent
                    auto file = processIt( sample, ms, writer.get() );
                    writer->insert_table( sample );
                }
            }
            break;

        case adcontrols::QuanSample::ProcessRawSpectra:
            if ( raw_ ) {
                adcontrols::MassSpectrum ms;
                size_t pos = 0;
                while ( ( pos = read_raw_spectrum( pos, raw_, ms ) ) ) {
                    if ( (*progress_)() )
                        return false;
                    auto file = processIt( sample, ms, writer.get(), false );       
                    writer->insert_table( sample );
                    sample.results().clear();
                } 
            }
            break;
        case adcontrols::QuanSample::ASIS:
            do {
                if ( auto folder = portfolio_->findFolder( L"Spectra" ) ) {
                    if ( auto folium = folder.findFoliumByName( sample.name() ) ) {
                        if ( fetch( folium ) ) {
                            if ( (*progress_)() )
                                return false;
                            adcontrols::MassSpectrumPtr ms;
                            if ( portfolio::Folium::get< adcontrols::MassSpectrumPtr >( ms, folium ) ) {
                                sample.name( folium.name().c_str() );
                                auto file = processIt( sample, *ms, writer.get() );
                                writer->insert_table( sample );
                            }
                        }
                    }
                }
            } while ( 0 );
            break;
        }
        processor_->complete( &sample );
    }
    QuanDocument::instance()->sample_processed( this );
    return true;
}


void
QuanSampleProcessor::open()
{
    try {
        datafile_.reset( adcontrols::datafile::open( path_, true ) );
        if ( datafile_ )
            datafile_->accept( *this );
    }
    catch ( ... ) { ADERROR() << boost::current_exception_diagnostic_information(); }
}

bool
QuanSampleProcessor::subscribe( const adcontrols::LCMSDataset& d )
{
    raw_ = &d;
    return true;
}

bool
QuanSampleProcessor::subscribe( const adcontrols::ProcessedDataset& d )
{
    portfolio_ = std::make_shared< portfolio::Portfolio >( d.xml() );
    return true;
}

bool
QuanSampleProcessor::fetch( portfolio::Folium& folium )
{
    try {
        folium = datafile_->fetch( folium.id(), folium.dataClass() );
        portfolio::Folio attachs = folium.attachments();
        for ( auto att : attachs ) {
            if ( att.empty() )
                fetch( att ); // recursive call make sure for all blongings load up in memory.
        }
    }
    catch ( std::bad_cast& ) {}
    return true;
}

size_t 
QuanSampleProcessor::read_first_spectrum( const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& ms, uint32_t tidx )
{
    size_t pos = 0;
    if ( tidx == uint32_t(-1) ) { // select last spectrum
        pos = raw->find_scan( -1, -1 ); // find last "safe" spectrum scan#
    } else {
        pos = raw->find_scan( tidx, -1 );
    }
    
    int idx, fcn, rep;
    if ( raw->index( pos, idx, fcn, rep ) ) {
        assert( fcn == 0 );
        if ( ( rep == 0 ) && raw->index( pos + 1, idx, fcn, rep ) )
            ++pos; // increment for skip rep = 0 data;
    }
    if ( raw->getSpectrum( -1, pos, ms ) ) {
        while ( raw->index( pos + 1, idx, fcn, rep ) && fcn == 0 ) {
            adcontrols::MassSpectrum a;
            if ( raw->getSpectrum( -1, ++pos, a ) )
                adcontrols::segments_helper::add( ms, a );
        }
        return pos + 1; // return next pos
    }
    return 0;
}

size_t 
QuanSampleProcessor::read_next_spectrum( size_t pos, const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& ms )
{
    int idx, fcn, rep;
    while ( raw->index( pos, idx, fcn, rep ) && (fcn != 0 || rep != 0) ) // find next protocol=0 aligned data
        ++pos;
    if ( fcn == 0 && rep == 0 ) {
        if ( raw->index( pos + 1, idx, fcn, rep ) ) 
            ++pos;
        if ( raw->getSpectrum( -1, pos, ms ) ) {
            while ( raw->index( pos + 1, idx, fcn, rep ) && fcn == 0 ) {
                adcontrols::MassSpectrum a;
                if ( raw->getSpectrum( -1, ++pos, a ) )
                    adcontrols::segments_helper::add( ms, a );
            }
            return pos + 1; // return next pos
        }
    }
    return 0;
}

size_t 
QuanSampleProcessor::read_raw_spectrum( size_t pos, const adcontrols::LCMSDataset * raw, adcontrols::MassSpectrum& ms )
{
    int idx, fcn, rep;
    while ( raw->index( pos, idx, fcn, rep ) && fcn != 0 )  // skip until 'fcn = 0' data 
        ++pos;
    if ( raw->getSpectrum( -1, pos, ms ) ) // read all corresponding segments
        return pos + 1;
    return 0;
}


bool 
QuanSampleProcessor::generate_spectrum( const adcontrols::LCMSDataset * raw
                                        , const adcontrols::QuanSample& sample
                                        , adcontrols::MassSpectrum& ms )
{

    auto range = std::make_pair( sample.scan_range_first(), sample.scan_range_second() );

    if ( (*progress_)() )
        return false;

    // select last data
    if ( range.first == uint32_t( -1 ) )
        return read_first_spectrum( raw, ms, range.first ) != 0;

    // anything else

    size_t pos = read_first_spectrum( raw, ms, range.first++ );
    if ( pos == 0 )
        return false; // no spectrum have read

    if ( nFcn_ == 1 )
        return true;  // no protocol/segmented data

    adcontrols::MassSpectrum a;
    while ( pos && range.first++ < range.second ) {
        if ( (*progress_)() )
            return false;
        if ( (pos = read_next_spectrum( pos, raw, a )) )
            adcontrols::segments_helper::add( ms, a );
    }

    return true;
}

// bool
// QuanSampleProcessor::processIt( adcontrols::QuanSample& sample
//                                 , const std::string& formula
//                                 , double mass
//                                 , std::shared_ptr< adcontrols::Chromatogram > chro
//                                 , std::shared_ptr< adcontrols::PeakResult > pkResult
//                                 , const std::wstring& dataGuid )
// {
//     // auto normalized_formula = cformula_->standardFormula( formula );
//     if ( auto pCompounds = procmethod_->find< adcontrols::QuanCompounds >() ) {
        
//         auto itCompound = std::find_if( pCompounds->begin(), pCompounds->end(), [formula] ( const adcontrols::QuanCompound& c ) {
//                 return c.formula() == formula;
//             } );

//         // Identify chromatographic peak

//         auto& pks = pkResult->peaks();        
//         if ( itCompound != pCompounds->end() && pks.size() > 0 ) {

//             auto pk = std::max_element( pks.begin(), pks.end()
//                                         , [] ( const adcontrols::Peak& a, const adcontrols::Peak& b ) { return a.peakHeight() < b.peakHeight(); } );
            
//             // assign peak name
//             pk->name( adportable::utf::to_wstring( formula ) );
            
//             adcontrols::QuanResponse resp;

//             resp.formula( formula.c_str() );
//             resp.uuid_cmpd( itCompound->uuid() );
//             resp.uuid_cmpd_table( pCompounds->uuid() );
//             resp.formula( itCompound->formula() );
//             resp.idx_ = pk->peakId();
//             resp.fcn_ = chro->fcn();
//             resp.mass_ = mass;
//             resp.intensity_ = pk->peakArea();
//             resp.amounts_ = 0;
//             resp.tR_ = double( adcontrols::timeutil::toMinutes( pk->peakTime() ) );
//             resp.dataGuid_ = dataGuid;

//             sample << resp;

//             return true;
//         }
//     }

//     return false;
// }

adfs::file
QuanSampleProcessor::processIt( adcontrols::QuanSample& sample
                                , adcontrols::MassSpectrum& profile
                                , QuanDataWriter * writer
                                , bool bSerialize )
{
    if ( auto pCentroidMethod = procmethod_->find< adcontrols::CentroidMethod >() ) {

        adcontrols::MassSpectrum centroid;
        adcontrols::MSPeakInfo pkInfo;
        adcontrols::MassSpectrum filtered;

        bool result(false);

        if ( pCentroidMethod->noiseFilterMethod() == adcontrols::CentroidMethod::eDFTLowPassFilter ) {
            filtered.clone( profile, true );
            for ( auto& ms : adcontrols::segment_wrapper<>( filtered ) ) {
                adcontrols::waveform::fft::lowpass_filter( ms, pCentroidMethod->cutoffFreqHz() );
                double base( 0 ), rms( 0 );
                const double * intens = ms.getIntensityArray();
                adportable::spectrum_processor::tic( uint32_t( ms.size() ), intens, base, rms );
                for ( size_t i = 0; i < ms.size(); ++i )
                    ms.setIntensity( i, intens[ i ] - base );
            }
            filtered.addDescription( adcontrols::description( L"process", dataproc::Constants::F_DFT_FILTERD ) );

            result = doCentroid( pkInfo, centroid, filtered, *pCentroidMethod );

        } else {
            result = doCentroid( pkInfo, centroid, profile, *pCentroidMethod );
        }

        if ( result ) {

            // doMSLock if required.
            if ( auto pCompounds = procmethod_->find< adcontrols::QuanCompounds >() ) {
                if ( auto lkMethod = procmethod_->find< adcontrols::MSLockMethod >() ) {
                    if ( lkMethod->enabled() )
                        doMSLock( pkInfo, centroid, *lkMethod, *pCompounds );
                }
            }
            
            // Look up compounds
            if ( auto pCompounds = procmethod_->find< adcontrols::QuanCompounds >() ) {
                if ( auto pTgtMethod = procmethod_->find< adcontrols::TargetingMethod >() ) {
                    doMSFind( pkInfo, centroid, sample, *pCompounds, *pTgtMethod );
                }
            }
            
            if ( bSerialize ) {
                adcontrols::MassSpectrum * pProfile = &profile;
                adcontrols::MassSpectrum * pFiltered = (filtered.size() > 0 ) ? &filtered : 0;
                adcontrols::MassSpectrum * pCentroid = &centroid;
                adcontrols::MSPeakInfo * pPkInfo = &pkInfo;
                if ( sample.channel() > 0 ) {
                    if ( auto p = profile.findProtocol( sample.channel() - 1 ) ) {
                        p->clearSegments();
                        pProfile = p;
                    }
                    if ( pFiltered ) {
                        if ( auto p = filtered.findProtocol( sample.channel() - 1 ) ) {
                            p->clearSegments();
                            pFiltered = p;

                        }
                    }
                    if ( auto p = centroid.findProtocol( sample.channel() - 1 ) ) {
                        p->clearSegments();
                        pCentroid = p;
                    }
                    if ( auto p = pkInfo.findProtocol( sample.channel() - 1 ) ) {
                        p->clearSegments();
                        pPkInfo = p;
                    }
                }

                std::lock_guard<std::mutex> lock( mutex_ );

                if ( adfs::file file = writer->write( *pProfile, sample.name() ) ) {

                    for ( auto& resp: sample.results() )
                        resp.dataGuid_ = file.name();
                    
                    if ( pFiltered )
                        writer->attach<adcontrols::MassSpectrum>( file, *pFiltered, dataproc::Constants::F_DFT_FILTERD );
                    
                    auto afile = writer->attach< adcontrols::MassSpectrum >( file, *pCentroid, dataproc::Constants::F_CENTROID_SPECTRUM );
                    
                    writer->attach< adcontrols::ProcessMethod >( afile, *procmethod_, L"ProcessMethod" );
                    writer->attach< adcontrols::MSPeakInfo >( file, *pPkInfo, dataproc::Constants::F_MSPEAK_INFO );
                    writer->attach< adcontrols::QuanSample >( file, sample, dataproc::Constants::F_QUANSAMPLE );
                    return file;
                }
            }
        }
    }
    return adfs::file();
}

bool
QuanSampleProcessor::doCentroid( adcontrols::MSPeakInfo& pkInfo
                                 , adcontrols::MassSpectrum& res
                                 , const adcontrols::MassSpectrum& profile
                                 , const adcontrols::CentroidMethod& m )
{
    adcontrols::CentroidProcess peak_detector;
    bool result = false;
    
    res.clone( profile, false );
    
    if ( peak_detector( m, profile ) ) {
        result = peak_detector.getCentroidSpectrum( res );
        pkInfo = peak_detector.getPeakInfo();
    }
    
    if ( profile.numSegments() > 0 ) {
        for ( size_t fcn = 0; fcn < profile.numSegments(); ++fcn ) {
            adcontrols::MassSpectrum centroid;
            result |= peak_detector( profile.getSegment( fcn ) );
            pkInfo.addSegment( peak_detector.getPeakInfo() );
            peak_detector.getCentroidSpectrum( centroid );
            res.addSegment( centroid );
        }
    }
    return result;
}

bool
QuanSampleProcessor::doMSLock( adcontrols::MSPeakInfo& pkInfo // will override
                               , adcontrols::MassSpectrum& centroid // will override
                               , const adcontrols::MSLockMethod& m
                               , const adcontrols::QuanCompounds& compounds )
{
    // find reference peak by mass window
    adcontrols::lockmass mslock;

    // TODO: consider how to handle segmented spectrum -- current impl is always process first 
    adcontrols::MSFinder find( m.tolerance( m.toleranceMethod() ), m.algorithm(), m.toleranceMethod() );

    for ( auto& compound : compounds ) {
        if ( compound.isLKMSRef() ) {
            double exactMass = cformula_->getMonoIsotopicMass( compound.formula() );
            size_t idx = find( centroid, exactMass );
            if ( idx != adcontrols::MSFinder::npos ) {
                // add found peaks into mslock
                mslock << adcontrols::lockmass::reference( compound.formula(), exactMass, centroid.getMass( idx ), centroid.getTime( idx ) );
            }
        }
    }

    if ( mslock.fit() ) {
        mslock( centroid, true );
        mslock( pkInfo, true );
        return true;
    }
    return false;
}

bool
QuanSampleProcessor::doMSFind( adcontrols::MSPeakInfo& pkInfo
                               , adcontrols::MassSpectrum& centroid
                               , adcontrols::QuanSample& sample
                               , const adcontrols::QuanCompounds& compounds
                               , const adcontrols::TargetingMethod& mtgt )
{
    double tolerance = mtgt.tolerance( adcontrols::idToleranceDaltons );

    adcontrols::segment_wrapper< adcontrols::MSPeakInfo > vPkInfo( pkInfo );
    adcontrols::segment_wrapper<> segs( centroid );

    int channel = sample.channel();
    int fcn = 0;
    for ( auto& fms : segs ) {

        if ( channel != 0 && fcn != (channel - 1) ) {
            ++fcn;
            continue;
        }
        
        auto& info = vPkInfo[ fcn ];
        adcontrols::MSFinder find( tolerance, adcontrols::idFindLargest, adcontrols::idToleranceDaltons );
        
        for ( auto& compound : compounds ) {
            adcontrols::QuanResponse resp;
            
            double exactMass = cformula_->getMonoIsotopicMass( compound.formula() );
            
            size_t idx = find( fms, exactMass );
            if ( idx != adcontrols::MSFinder::npos ) {
                
                resp.uuid_cmpd( compound.uuid() );
                resp.uuid_cmpd_table( compounds.uuid() );
                resp.formula( compound.formula() );
                resp.idx_ = int32_t(idx);
                resp.fcn_ = fcn;
                resp.mass_ = fms.getMass( idx );
                resp.intensity_ = fms.getIntensity( idx );
                resp.amounts_ = 0;
                resp.tR_ = 0;

                using adcontrols::annotation;
                annotation anno( resp.formula(), resp.mass_, resp.intensity_, resp.idx_, resp.intensity_, annotation::dataFormula );
                fms.get_annotations() << anno;

                (info.begin() + idx)->formula( resp.formula() );

                sample << resp;
            }
        }
        ++fcn;
    }
    return false;
}

