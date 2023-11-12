/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "quanchromatogram.hpp"
#include "quanchromatograms.hpp"
#include "quandatawriter.hpp"
#include "document.hpp"
#include "quanprocessor.hpp"
#include "quansampleprocessor.hpp"
#include "quantarget.hpp"
#include "../plugins/dataproc/dataprocconstants.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/histogram.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quan/extract_by_mols.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quanresponse.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/file.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adlog/logger.hpp>
#include <adportable/date_time.hpp>
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adprocessor/autotargeting.hpp>
#include <adprocessor/autotargeting.hpp>
#include <adprocessor/autotargetingcandidates.hpp>
#include <adprocessor/autotargetingcandidates.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adprocessor/mschromatogramextractor.hpp>
#include <adutils/cpio.hpp>
#include <adwidgets/progressinterface.hpp>
//#include <adwidgets/progresswnd.hpp>
#include <chromatogr/chromatography.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <algorithm>
#include <filesystem>
#include <regex>
#include <map>

namespace quan {

    struct targeting_value {
        double matchedMass;
        double mass_error;
        int32_t idx;
        targeting_value()
            : matchedMass( 0 ), mass_error( 0 ), idx( 0 ) {
        }
        targeting_value( double mass, double error, int32_t i )
            : matchedMass( mass ), mass_error( error ), idx( i ) {
        }
    };

    void tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const targeting_value& t ) {
        jv = {{ "matchedMass", t.matchedMass }
            , { "mass_error", t.mass_error }
            , { "idx", t.idx }
        };
    }

    targeting_value tag_invoke( boost::json::value_to_tag< targeting_value >&, const boost::json::value& jv ) {
        if ( jv.kind() == boost::json::kind::object ) {
            targeting_value t;
            using namespace adportable::json;
            auto obj = jv.as_object();
            extract( obj, t.matchedMass , "matchedMass" );
            extract( obj, t.mass_error  , "mass_error"  );
            extract( obj, t.idx         , "idx"         );
            return t;
        }
        return {};
    }


    struct target_result_finder {

        static bool find( const adcontrols::Targeting& t
                          , const std::string& formula
                          , int32_t proto
                          , const adcontrols::MassSpectrum& centroid
                          , targeting_value& tv ) {

            assert( centroid.isCentroid() );

            auto it = std::find_if( t.candidates().begin(), t.candidates().end()
                                    , [&](const auto& c){ return c.fcn == proto && c.formula == formula; });

            if ( it != t.candidates().end() ) {
                try {
                    auto& tms = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( centroid )[ proto ];
                    tv = targeting_value( tms.mass( it->idx ), (it->mass - it->exact_mass), it->idx );
                    return true;
                } catch ( std::exception& ex ) {
                    ADDEBUG() << "Exception: " << ex.what();
                }
            }
            ADDEBUG() << "no targget candidate found";
            return false;
        }
    };

    struct annotation {
        static void add( adcontrols::MassSpectrum& centroid, int idx, int proto, const std::string& formula ) {
            if ( auto tms = centroid.findProtocol( proto ) ) {
                // todo: erase if peak already has the annotation
                tms->get_annotations() << adcontrols::annotation( formula
                                                                  , tms->mass( idx )
                                                                  , tms->intensity( idx )
                                                                  , idx, 0, adcontrols::annotation::dataFormula );
            }
        }
    };

    // new interface as of 2018-MAY
    struct save_chromatogram {

        static std::wstring make_title( const wchar_t * dataSource, const adcontrols::Chromatogram& c ) {
            std::filesystem::path path( dataSource );

            auto extract_by_mols = boost::json::value_to< adcontrols::quan::extract_by_mols >(
                adportable::json_helper::find( c.generatorProperty(), "generator.extract_by_mols" ) );

            auto wform = extract_by_mols.wform_type;

            const auto& mol = extract_by_mols.moltable_;
            auto formula = adportable::utf::to_wstring( mol.formula );

            return ( boost::wformat( L"%s #%d W(%.1fmDa) {%s}-%s" )
                     % formula % mol.protocol % ( mol.width * 1000 ) % path.stem().wstring()
                     % adportable::utf::to_wstring( wform ) ).str();
        }

        static boost::uuids::uuid
        save( std::shared_ptr< QuanDataWriter > writer, const wchar_t * dataSource
              , std::pair< std::shared_ptr< adcontrols::Chromatogram >, std::shared_ptr< adcontrols::PeakResult > >& pair
              , const adcontrols::ProcessMethod& procm, size_t idx )   {

            auto title = make_title( dataSource, *pair.first );
            if ( adfs::file file = writer->write( *pair.first, title ) ) {
                auto fGuid = boost::uuids::string_generator()( file.name() );
                // pair.first->ptree().put( "folder.dataGuid", fGuid );
                boost::json::object jobj;
                if ( auto prop = pair.first->generatorProperty() ) {
                    auto jv = boost::json::parse( *prop );
                    jobj = jv.as_object();
                }
                jobj[ "folder" ] = boost::json::object{{ "dataGuid", boost::uuids::to_string( fGuid ) }};
                pair.first->setGeneratorProperty( boost::json::serialize( jobj ));
                //----

                auto afile = writer->attach< adcontrols::PeakResult >( file, *pair.second, pair.second->dataClass() );
                writer->attach< adcontrols::ProcessMethod >( file, procm, L"Process Method" );
                return fGuid;
            }
            return {{ 0 }};
        }
    };

    // new interface as of 2018-MAY
    struct save_spectrum {

        static std::wstring make_title( const wchar_t * dataSource, const std::string& formula, double tR, const wchar_t * trailer = L"" ) {
            std::filesystem::path path( dataSource );
            return ( boost::wformat( L"%s tR(%.3fs) {%s} %s" ) % adportable::utf::to_wstring( formula ) % tR %  path.stem().wstring() % trailer ).str();
        }

        static boost::uuids::uuid
        save( std::shared_ptr< QuanDataWriter > writer
              , const wchar_t * dataSource
              , std::shared_ptr< const adcontrols::MassSpectrum > ms
              , const adcontrols::MassSpectrum& centroid // added 2020-11-01
              , const adcontrols::MSPeakInfo& pkinfo  // added 2020-11-01
              , std::shared_ptr< const adcontrols::Targeting > targeting  // added 2020-11-01
              , const std::wstring& title
              , const adcontrols::ProcessMethod& procm
              , const std::string& formula
              , int32_t proto ) {

            if ( auto file = writer->write( *ms, title ) ) {
                if ( auto att = writer->attach< adcontrols::MSPeakInfo >( file, pkinfo, dataproc::Constants::F_MSPEAK_INFO ) ) {
                }
                if ( auto att = writer->attach< adcontrols::ProcessMethod >( file, procm, L"Process Method" ) ) {
                }
                if ( auto att = writer->attach< adcontrols::MassSpectrum >( file, centroid, adcontrols::constants::F_CENTROID_SPECTRUM ) ) {
                    if ( targeting )
                        writer->attach< adcontrols::Targeting >( att, *targeting, adcontrols::constants::F_TARGETING );
                }
                if ( ms->isHistogram() ) {
                    if ( auto hist = adcontrols::histogram::make_profile( *ms ) )
                        writer->attach< adcontrols::MassSpectrum >( file, *hist, adcontrols::constants::F_PROFILED_HISTOGRAM );
                }
                return boost::uuids::string_generator()( file.name() );
            }
            return {{ 0 }};
        }
    };

    /////////////////////////
    /////////////////////////

    struct response_builder {

        static void add( QuanSampleProcessor& processor
                         , adcontrols::QuanSample& sample
                         , const std::pair< std::shared_ptr< adcontrols::Chromatogram >, std::shared_ptr< adcontrols::PeakResult> >& pair
                         , const adcontrols::QuanCompounds& compounds ) {

            using namespace adportable;

            boost::json::object jobj;
            jobj = json_helper::parse( pair.first->generatorProperty() ).as_object();

            auto targeting = adportable::json_helper::find( jobj, "targeting" );
            if ( targeting.is_null() ) {
                targeting = adportable::json_helper::find( jobj, "generator.extract_by_mols.auto_target_candidate" );
            }

            auto matchedMass = json_helper::value_to< double >( targeting, "matchedMass" );
            auto dataGuid = json_helper::value_to< boost::uuids::uuid >( jobj, "folder.dataGuid" );
            auto extract_by_mols = json_helper::find( jobj, "generator.extract_by_mols" );
            auto cmpdGuid = json_helper::value_to< boost::uuids::uuid >( extract_by_mols, "molid" );
            auto mol = json_helper::find( extract_by_mols, "moltable" );
            if ( mol.is_object() ) {
                if ( auto formula = json_helper::value_to< std::string >( mol, "formula" ) ) {
                    adcontrols::QuanResponse resp;
                    resp.uuid_cmpd( cmpdGuid.get() );  // compound id (uuid) identify each molecule(formula) and protocol
                    resp.uuid_cmpd_table( compounds.uuid() );
                    if ( dataGuid )
                        resp.setDataGuid( dataGuid.get() );   // corresponding chromatogram data on output adfs file
                    resp.setMass( *matchedMass );
                    resp.setPeakIndex( -1 );
                    auto it = std::find_if( pair.second->peaks().begin()
                                            , pair.second->peaks().end()
                                            , [&](auto& p){ return p.formula() == formula.get(); } );
                    if ( it != pair.second->peaks().end() ) {
                        resp.formula( formula.get().c_str() );
                        resp.setPeakIndex( it->peakId() );
                        resp.setFcn( pair.first->protocol() );
                        resp.setIntensity( it->peakArea() );
                        resp.setAmounts( 0 );
                        resp.set_tR( it->peakTime() );
                        resp.setPkArea( it->peakArea() );
                        resp.setPkHeight( it->peakHeight() );
                        resp.setPkWidth( it->peakWidth() );
                        resp.setTheoreticalPlate( it->theoreticalPlate().ntp() );
                        resp.setAsymmetry( it->asymmetry().asymmetry() );
                        resp.setResolution( it->resolution().resolution() );
                    }
                    sample << resp;

                    jobj[ "resp" ] = boost::json::object{
                        { "uuid_cmpd", boost::uuids::to_string( resp.uuid_cmpd() ) }
                        , { "uuid_cmpd_table", boost::uuids::to_string( resp.uuid_cmpd_table() ) }
                        , { "dataGuid", boost::uuids::to_string( dataGuid.get() ) }
                        , { "mass", resp.mass() }
                        , { "idx", resp.peakIndex() }
                        , { "fcn", resp.fcn() }
                        , { "intensity", resp.intensity() }
                        , { "tR", resp.tR() }
                    };
                    pair.first->setGeneratorProperty( boost::json::serialize( jobj ) );
                } else {
                    ADDEBUG() << "### no formula found ###";
                }
            } else {
                ADDEBUG() << "### no moltable found ###";
            }

#if ! defined NDEBUG // || 1
            ADDEBUG() << jobj;
#endif
        }
    };

    /////////////////////////
    /////////////////////////
}

using namespace quan;

// static
std::wstring
QuanChromatogramProcessor::make_title( const wchar_t * dataSource, const QuanCandidate& candidate, const wchar_t * trailer )
{
    std::filesystem::path path( dataSource );
    std::wstring title = ( boost::wformat( L"%s, %s" ) % path.stem().wstring() % adportable::utf::to_wstring( candidate.formula() ) ).str();
    title += trailer;  //L" (1st phase)";

    return title;
}

std::wstring
QuanChromatogramProcessor::make_title( const wchar_t * dataSource, const std::string& formula, int fcn, double error, const wchar_t * trailer )
{
    return L"";
}


QuanChromatogramProcessor::QuanChromatogramProcessor( std::shared_ptr< const adcontrols::ProcessMethod > pm )
    : debug_level_( 0 )
    , save_on_datasource_( false )
    , procm_( std::make_shared< adcontrols::ProcessMethod >( *pm ) )
    , cXmethods_{{ std::make_unique< adcontrols::MSChromatogramMethod >(), std::make_unique< adcontrols::MSChromatogramMethod >() }}
{
    if ( auto qm = pm->find< adcontrols::QuanMethod >() ) {
        debug_level_ = qm->debug_level();
        save_on_datasource_ = qm->save_on_datasource();
    }

    if ( auto pCompounds = pm->find< adcontrols::QuanCompounds >() ) {

        // mass chromatograms extraction method
        pCompounds->convert_if( cXmethods_[ 0 ]->molecules(), []( const adcontrols::QuanCompound& comp ){ return !comp.isCounting();} );
        pCompounds->convert_if( cXmethods_[ 1 ]->molecules(), []( const adcontrols::QuanCompound& comp ){ return comp.isCounting();} );

        // override 'auto targeting befor XIC generation' parameter
        auto qrm = pm->find< adcontrols::QuanResponseMethod >();
        auto lkm = pm->find< adcontrols::MSLockMethod >();
        auto targeting_method = pm->find< adcontrols::TargetingMethod >();

        for ( auto& cm: cXmethods_ ) {
            if ( qrm ) {
                cm->setEnableAutoTargeting( qrm->enableAutoTargeting() );
                cm->setPeakWidthForChromatogram( qrm->peakWidthForChromatogram() );
            }
            if ( lkm ) {
                cm->setLockmass( lkm->enabled() );
            }
            if ( targeting_method )
                cm->width( targeting_method->tolerance( targeting_method->toleranceMethod() ), adcontrols::MSChromatogramMethod::widthInDa );
        }

        if ( targeting_method ) {
            auto tm = std::make_shared< adcontrols::TargetingMethod >( *targeting_method );
            pCompounds->convert( tm->molecules() );
            *procm_ *= *tm;
        }
    }
}

////////////////////////////////////
/// main entry point             ///
/// new interface as of 2018-MAY ///
///////////////////////////////////
bool
QuanChromatogramProcessor::operator()( QuanSampleProcessor& processor
                                       , adcontrols::QuanSample& sample
                                       , std::shared_ptr< QuanDataWriter > writer
                                       , std::shared_ptr< adwidgets::ProgressInterface > progress )
{
    if ( auto raw = processor.rawdata() ) { //getLCMSDataset() ) {

        if ( raw->dataformat_version() < 3 )  // no support for old (before 2014) data
            return false;

        auto extractor = std::make_unique< adprocessor::v3::MSChromatogramExtractor >( raw, &processor );
        std::array< std::shared_ptr< const adcontrols::DataReader >, 2 > readers;

        for ( auto reader: raw->dataReaders() ) {
            if ( ( reader->objtext().find( "waveform" ) != std::string::npos ) ||                                    // soft average
                 std::regex_search( reader->objtext(), std::regex( "^[1-9]\\.u5303a\\.ms-cheminfo.com" ) ) ) {       // hard average
                readers[ 0 ] = reader;
            }

            if ( ( reader->objtext().find( "histogram" ) != std::string::npos ) ||                                   // soft counting
                 std::regex_search( reader->objtext(), std::regex( "^pkd\\.[1-9]\\.u5303a\\.ms-cheminfo.com" ) ) ) { // hard counting
                readers[ 1 ] = reader;
            }
        }
        if ( !readers[ 0 ] && !readers[ 1 ] ) {
            ADDEBUG() << "no data readers found";
            return false;
        }

        auto pCompounds = procm_->find< adcontrols::QuanCompounds >();
        if ( !pCompounds ) {
            return false;
        }
        sample.set_time_of_injection( extractor->time_of_injection() );


        size_t idx = 0;// 0 := avg, 1 := pkd
        for ( auto reader: readers ) {
            if ( reader ) {
                adcontrols::ProcessMethod pm( *procm_ );
                pm *= (*cXmethods_[ idx ]);
                bool autoTargeting( false );
                if ( auto cm = pm.find< adcontrols::MSChromatogramMethod >() ) {
                    autoTargeting = cm->enableAutoTargeting();
                }
                if ( autoTargeting )
                    extract_chromatograms_via_auto_target( processor, sample, writer, idx, pm, *pCompounds, *extractor, reader, progress);
                else
                    extract_chromatograms_via_mols( processor, sample, writer, idx, pm, *pCompounds, *extractor, reader, progress);
            }
            ++idx;
        }
    }
	return true;
}

// static
bool
QuanChromatogramProcessor::findPeaks( adcontrols::PeakResult& res, const adcontrols::Chromatogram& chr, const adcontrols::PeakMethod& pm )
{
    chromatogr::Chromatography peakfinder( pm );
    if ( peakfinder( chr ) ) {
        res.baselines() = peakfinder.getBaselines();
        res.peaks() = peakfinder.getPeaks();
        return true;
    }
    return false;
}

// static
bool
QuanChromatogramProcessor::identify( adcontrols::PeakResult& res, const adcontrols::QuanCompounds& compounds, const adcontrols::Chromatogram& chr )
{
    auto extract_by_mols = adportable::json_helper::find( chr.generatorProperty(), "generator.extract_by_mols" );
    if ( extract_by_mols.is_object() ) {
        if ( auto molid = adportable::json_helper::value_to< boost::uuids::uuid >( extract_by_mols, "molid" ) ) {
            auto mol = adportable::json_helper::find( extract_by_mols, "moltable" );
            auto formula = adportable::json_helper::value_to< std::string >( mol, "formula" );

            auto cmpd = std::find_if( compounds.begin(), compounds.end(), [&]( auto& a ) { return a.uuid() == *molid; } );
            while ( cmpd != compounds.end() ) {
                auto pk = std::find_if( res.peaks().begin(), res.peaks().end()
                                        , [&]( const auto& p ){ return p.startTime() < cmpd->tR() && cmpd->tR() < p.endTime(); } );
                if ( pk != res.peaks().end() ) {
                    pk->setFormula( formula.get().c_str() );
                    pk->setName( adcontrols::ChemicalFormula::formatFormula( pk->formula() ) );
                }

                // next candidate
                std::advance( cmpd, 1 );
                cmpd = std::find_if( cmpd, compounds.end(), [&]( auto& a ) { return a.uuid() == *molid; } );
            }
        }
        return true;
    }
	return false;
}

// static
void
QuanChromatogramProcessor::extract_chromatograms_via_auto_target( QuanSampleProcessor& processor
                                                                  , adcontrols::QuanSample& sample
                                                                  , std::shared_ptr< QuanDataWriter > writer
                                                                  , size_t idx
                                                                  , const adcontrols::ProcessMethod& pm
                                                                  , const adcontrols::QuanCompounds& cmpds
                                                                  , adprocessor::v3::MSChromatogramExtractor& extractor
                                                                  , std::shared_ptr< const adcontrols::DataReader > reader
                                                                  , std::shared_ptr< adwidgets::ProgressInterface > progress )
{
    std::vector< adprocessor::AutoTargetingCandidates > candidates;
    if ( auto cm = pm.find< adcontrols::MSChromatogramMethod >() ) {
        for ( auto& mol: cm->molecules().data() ) {
            if ( mol.enable() && mol.tR() ) {
                int proto = mol.protocol() ? *mol.protocol() : 0;
                candidates.emplace_back( adprocessor::AutoTargeting().doit( proto, mol, pm, reader, []( auto& lock ){} ) );
            }
        }
    }
    // validation
    for ( const auto& candidate: candidates ) {
        ADDEBUG() << "candidate: " << candidate.mol().formula() << ", size: " << candidate.size();
    }
    // <--

    std::vector< std::pair< std::shared_ptr< adcontrols::Chromatogram >
                            , std::shared_ptr< adcontrols::PeakResult > > > rlist;

    do {
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > clist;
        extractor.extract_by_mols( clist, pm, reader, candidates, [progress]( size_t, size_t )->bool{ return (*progress)(); } );
        std::transform( clist.begin(), clist.end(), std::back_inserter( rlist )
                        , []( auto p ){ return std::make_pair( std::move( p ), std::make_shared< adcontrols::PeakResult >() ); });
    } while (0);

    // detect chromatographic peaks
    if ( auto peakm = pm.find< adcontrols::PeakMethod >() ) {
        for ( auto& pair: rlist ) {
            if ( findPeaks( *pair.second, *pair.first, *peakm ) )
                identify( *pair.second, cmpds, *pair.first );
            pair.first->setBaselines( pair.second->baselines() );
            pair.first->setPeaks( pair.second->peaks() );          // copy identified peak result (for annotation in chroamtogram widget)
        }
    }

    // load spectrum corresponding to chromatographic peak
    // rlist.size() == mols.size()
    std::map< boost::uuids::uuid, boost::uuids::uuid > msGuids;
    std::map< boost::uuids::uuid, int > indices;

    for ( auto& c: candidates ) {
        double tR = c.mol().tR() ? *c.mol().tR() : 0;
        auto title = save_spectrum::make_title( sample.dataSource(), c.mol().formula(), tR, (idx ? L" (histogram)" : L" (profile)" ) );
        auto molid = c.mol().molid(); // property< boost::uuids::uuid >( "molid" );
        auto msGuid = save_spectrum::save( writer
                                           , sample.dataSource()
                                           , c.refms()
                                           , *c.refms_processed()
                                           , *c.refms_pkinfo()
                                           , c.targeting()
                                           , title
                                           , pm
                                           , c.mol().formula()
                                           , c.mol().protocol() ? c.mol().protocol().get() : 0 );
        if ( molid ) {
            msGuids[ *molid ] = msGuid;
            if ( c.size() )
                indices[ *molid ] = c[0]->idx;
        }
    }

    for ( auto& pair: rlist ) {
        auto& chr = pair.first;
        boost::uuids::uuid molid;
        {
            auto jv = adportable::json_helper::find( chr->generatorProperty(), "generator.extract_by_mols.molid" );
            if ( !jv.is_null() ) {
                // molid = boost::json::value_to< boost::uuids::uuid >( jv );
                molid = boost::lexical_cast< boost::uuids::uuid >( jv.as_string().data() );
            }
        }

        auto dataGuid = save_chromatogram::save( writer, sample.dataSource(), pair, pm, idx );
        response_builder::add( processor, sample, pair, cmpds );

        size_t index = indices[ molid ];
        auto jv = adportable::json_helper::find( chr->generatorProperty(), "generator.extract_by_mols.auto_target_candidate" );
        if ( jv.is_object() ) {
            if ( auto idx = adportable::json_helper::value_to< size_t >( jv, "idx" ) ) {
                assert( index == *idx );
            }
        }
        writer->insert_reference( dataGuid, msGuids[ molid ], index, chr->protocol() );
    }
}


void
QuanChromatogramProcessor::extract_chromatograms_via_mols( QuanSampleProcessor& processor
                                                           , adcontrols::QuanSample& sample
                                                           , std::shared_ptr< QuanDataWriter > writer
                                                           , size_t idx
                                                           , const adcontrols::ProcessMethod& pm
                                                           , const adcontrols::QuanCompounds& cmpds
                                                           , adprocessor::v3::MSChromatogramExtractor& extractor
                                                           , std::shared_ptr< const adcontrols::DataReader > reader
                                                           , std::shared_ptr< adwidgets::ProgressInterface > progress )
{
    // ADDEBUG() << "## " << __FUNCTION__ << " ## " << reader->objtext();
    std::vector< std::pair< std::shared_ptr< adcontrols::Chromatogram >
                            , std::shared_ptr< adcontrols::PeakResult > > > rlist;

    do {
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > clist;
        extractor.extract_by_mols( clist, pm, reader, [progress]( size_t, size_t )->bool{ return (*progress)(); } );
        std::transform( clist.begin(), clist.end(), std::back_inserter( rlist )
                        , []( auto p ){ return std::make_pair( std::move( p ), std::make_shared< adcontrols::PeakResult >() ); });
    } while (0);

    // detect chromatographic peaks
    if ( auto peakm = pm.find< adcontrols::PeakMethod >() ) {
        for ( auto& pair: rlist ) {
            if ( findPeaks( *pair.second, *pair.first, *peakm ) )
                identify( *pair.second, cmpds, *pair.first );
            pair.first->setBaselines( pair.second->baselines() );
            pair.first->setPeaks( pair.second->peaks() );          // copy identified peak result (for annotation in chroamtogram widget)
        }
    }

    // load spectrum corresponding to chromatographic peak
    for ( auto& pair: rlist ) {
        boost::uuids::uuid msGuid{{ 0 }}, dataGuid{{ 0 }};
        auto& chr = pair.first;
        // auto ptree = chr->ptree();
        for ( auto& pk: pair.second->peaks() ) {
            if ( !pk.name().empty() ) {
                if ( auto ms = extractor.getMassSpectrum( pk.peakTime() ) ) {
                    // save corresponding spectrum
                    auto title = save_spectrum::make_title( sample.dataSource()
                                                            , pk.formula()
                                                            , pk.peakTime()
                                                            , (idx == 0 ? L" (avg)" : L" (pkd)" ) );

                    std::shared_ptr< adcontrols::MassSpectrum > centroid;
                    std::shared_ptr< adcontrols::MSPeakInfo > pkinfo;
                    std::unique_ptr< adcontrols::Targeting > targeting;

                    if ( auto pkd = adprocessor::dataprocessor::doCentroid( *ms, pm ) ) {
                        std::tie( pkinfo, centroid ) = *pkd;
                        centroid->addDescription( adcontrols::description( L"process", L"Centroid" ) );

                        if ( auto tm = pm.find< adcontrols::TargetingMethod >() ) {
                            targeting = std::make_unique< adcontrols::Targeting > ( *tm );
                            if ( targeting->force_find( *centroid, pk.formula(), chr->protocol() ) ) {
                                targeting_value tv;
                                if ( target_result_finder::find( *targeting, pk.formula(), chr->protocol(), *centroid, tv /* ptree */ ) ) {
                                    annotation::add( *centroid, tv.idx, chr->protocol(), pk.formula() );
                                    auto jv = boost::json::value_from( tv );
                                    auto prop = adportable::json_helper::parse( chr->generatorProperty() );
                                    if ( prop.is_object() )
                                        prop.as_object()[ "targeting" ] = jv;
                                    else
                                        prop = {{ "targeting", jv }};
                                    chr->setGeneratorProperty( boost::json::serialize( prop ) );
                                }
                            }
                        }
                    }

                    msGuid = save_spectrum::save( writer
                                                  , sample.dataSource()
                                                  , ms
                                                  , *centroid
                                                  , *pkinfo
                                                  , std::move( targeting )
                                                  , title
                                                  , pm
                                                  , pk.formula()
                                                  , chr->protocol() );
                }
            }
        }
        dataGuid = save_chromatogram::save( writer, sample.dataSource(), pair, pm, idx );
        // ADDEBUG() << "----------- add --------------\n" << pair.first->ptree();
        response_builder::add( processor, sample, pair, cmpds );
        auto idx = adportable::json_helper::value_to< int32_t >( adportable::json_helper::parse( chr->generatorProperty() ), "targeting.idx" );
        writer->insert_reference( dataGuid, msGuid, idx ? *idx : -1, chr->protocol() );
    }
}
