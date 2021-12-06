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
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quanresponse.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adprocessor/autotargeting.hpp>
#include <adprocessor/autotargetingcandidates.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/file.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/date_time.hpp>
#include <adportable/float.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adprocessor/mschromatogramextractor.hpp>
#include <adprocessor/autotargeting.hpp>
#include <adprocessor/autotargetingcandidates.hpp>
#include <adutils/cpio.hpp>
//#include <adwidgets/progresswnd.hpp>
#include <adwidgets/progressinterface.hpp>
#include <chromatogr/chromatography.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/json.hpp>
#include <algorithm>
#include <regex>
#include <map>

namespace quan {

    struct target_result_finder {

        static bool find( const adcontrols::Targeting& t, const std::string& formula, int32_t proto
                          , const adcontrols::MassSpectrum& centroid, boost::property_tree::ptree& ptree ) {

            assert( centroid.isCentroid() );

#if !defined NDEBUG
            for ( auto& c : t.candidates() )
                ADDEBUG() << "target_finder: " << c.formula << " == " << formula << ", " << ( c.formula == formula ) << ", mass: " << c.mass;
#endif
            auto it = std::find_if( t.candidates().begin(), t.candidates().end()
                                    , [&](const auto& c){ return c.fcn == proto && c.formula == formula; });

            if ( it != t.candidates().end() ) {
                try {
                    auto& tms = adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( centroid )[ proto ];
                    ptree.put( "targeting.matchedMass", tms.mass( it->idx ) );
                    ptree.put( "targeting.mass_error", (it->mass - it->exact_mass) );
                    ptree.put( "targeting.idx", it->idx );
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
                tms->get_annotations() << adcontrols::annotation( formula, tms->getMass( idx ), tms->getIntensity( idx ), idx, 0, adcontrols::annotation::dataFormula );
            }
        }
    };

    // new interface as of 2018-MAY
    struct save_chromatogram {

        static std::wstring make_title( const wchar_t * dataSource, const boost::property_tree::ptree& pt )  {
            boost::filesystem::path path( dataSource );

            auto wform = pt.get_optional< std::string >( "generator.extract_by_mols.wform_type" );

            if ( auto mol = pt.get_child_optional( "generator.extract_by_mols.moltable" ) ) {
                auto formula = mol.get().get_optional< std::string >( "formula" );
                auto width = mol.get().get_optional< double >( "width" );
                auto proto = mol.get().get_optional< int32_t >( "protocol" );

                return ( boost::wformat( L"%s #%d W(%.1fmDa) {%s}-%s" )
                         % ( formula ? adportable::utf::to_wstring( formula.get() ) : L"" )
                         % ( proto ? proto.get() : (-1) )
                         % ( width ? width.get() *1000 : 0.0 )
                         % path.stem().wstring()
                         % ( wform ? adportable::utf::to_wstring( wform.get() ) : L"n/a" ) ).str();
            } else {
                return ( boost::wformat( L"{%s}" ) % path.stem().wstring() ).str();
            }
        }

        static boost::uuids::uuid
        save( std::shared_ptr< QuanDataWriter > writer, const wchar_t * dataSource
              , std::pair< std::shared_ptr< adcontrols::Chromatogram >, std::shared_ptr< adcontrols::PeakResult > >& pair
              , const adcontrols::ProcessMethod& procm, size_t idx )   {

            auto title = make_title( dataSource, pair.first->ptree() );
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
            boost::filesystem::path path( dataSource );
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

            auto ptree = pair.first->ptree();
            boost::json::object jobj;
            if ( auto prop = pair.first->generatorProperty() ) {
                auto jv = boost::json::parse( *pair.first->generatorProperty() );
                jobj = jv.as_object();
            }

            auto matchedMass = ptree.get_optional< double >( "targeting.matchedMass" );
            auto dataGuid = ptree.get_optional< boost::uuids::uuid >( "folder.dataGuid" );

            if ( auto child = ptree.get_child_optional( "generator.extract_by_mols" ) ) {

                if ( auto cmpdGuid = child.get().get_optional< boost::uuids::uuid >( "molid" ) ) { // "generator.extract_by_mols.molid"

                    if ( auto mol = child.get().get_child_optional( "moltable" ) ) {               // "generator.extract_by_mols.moltable"

                        // lookup c-peak
                        if ( auto formula = mol.get().get_optional< std::string >( "formula" ) ) {
                            adcontrols::QuanResponse resp;
                            resp.uuid_cmpd( cmpdGuid.get() );  // compound id (uuid) identify each molecule(formula) and protocol
                            resp.uuid_cmpd_table( compounds.uuid() );
                            if ( dataGuid )
                                resp.setDataGuid( dataGuid.get() );   // corresponding chromatogram data on output adfs file
                            resp.setMass( matchedMass ? matchedMass.get() : 0 );
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
                            assert( 0 );
                        }
                    }
                } else {
                    ADERROR() << "No Compound ID found -- internal error";
                }
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
    boost::filesystem::path path( dataSource );
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
    if ( auto raw = processor.getLCMSDataset() ) {

        if ( raw->dataformat_version() < 3 )  // no support for old (before 2014) data
            return false;

        auto extractor = std::make_unique< adprocessor::v3::MSChromatogramExtractor >( raw );
        std::array< std::shared_ptr< const adcontrols::DataReader >, 2 > readers;

        for ( auto reader: raw->dataReaders() ) {
            if ( ( reader->objtext().find( "waveform" ) != std::string::npos ) ||  // soft average
                 std::regex_search( reader->objtext(), std::regex( "^[1-9]\\.u5303a\\.ms-cheminfo.com" ) ) ) {  // hard average
                readers[ 0 ] = reader;
            }

            if ( ( reader->objtext().find( "histogram" ) != std::string::npos ) ||  // soft counting
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
    if ( auto child = chr.ptree().get_child_optional( "generator.extract_by_mols" ) ) {
        auto uuid = child.get().get_optional< boost::uuids::uuid >( "molid" );
        if ( auto mol = child.get().get_child_optional( "moltable" ) ) {
            auto formula = mol.get().get_optional< std::string >( "formula" );

            auto cmpd = std::find_if( compounds.begin(), compounds.end(), [&]( auto& a ) { return a.uuid() == uuid.get(); } );
            while ( cmpd != compounds.end() ) {

                auto pk = std::find_if( res.peaks().begin(), res.peaks().end(), [&]( const auto& p ){ return p.startTime() < cmpd->tR() && cmpd->tR() < p.endTime(); } );
                if ( pk != res.peaks().end() ) {
                    pk->setFormula( formula.get().c_str() );
                    pk->setName( adcontrols::ChemicalFormula::formatFormula( pk->formula() ) );
                }

                // next candidate
                std::advance( cmpd, 1 );
                cmpd = std::find_if( cmpd, compounds.end(), [&]( auto& a ) { return a.uuid() == uuid.get(); } );
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
    ADDEBUG() << "## " << __FUNCTION__ << " ## " << reader->objtext();

    std::vector< adprocessor::AutoTargetingCandidates > candidates;
    if ( auto cm = pm.find< adcontrols::MSChromatogramMethod >() ) {
        for ( auto& mol: cm->molecules().data() ) {
            if ( mol.enable() && mol.tR() ) {
                int proto = mol.protocol() ? *mol.protocol() : 0;
                candidates.emplace_back( adprocessor::AutoTargeting().doit( proto, mol, pm, reader, []( auto& lock ){} ) );
            }
        }
    }

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
        auto molid = c.mol().property< boost::uuids::uuid >( "molid" );
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
            // if ( !jv.is_null() )
            //     molid = boost::json::value_to< boost::uuids::uuid >( jv );
            molid = boost::lexical_cast< boost::uuids::uuid >( jv.as_string().data() );
            ADDEBUG() << "############## value_to< boost::uuids::uuid > = " << molid;
        }

        auto dataGuid = save_chromatogram::save( writer, sample.dataSource(), pair, pm, idx );
        response_builder::add( processor, sample, pair, cmpds );

        size_t index = indices[ molid ];
        auto jv = adportable::json_helper::find( chr->generatorProperty(), "generator.extract_by_mols.auto_target_candidate" );
        if ( jv.is_object() ) {
            if ( auto idx = adportable::json_helper::value< size_t >( jv, "idx" ) ) {
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
    ADDEBUG() << "## " << __FUNCTION__ << " ## " << reader->objtext();

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
        auto ptree = chr->ptree();
        for ( auto& pk: pair.second->peaks() ) {
            if ( !pk.name().empty() ) {
                if ( auto ms = extractor.getMassSpectrum( pk.peakTime() ) ) {
                    // save corresponding spectrum
                    auto title = save_spectrum::make_title( sample.dataSource(), pk.formula(), pk.peakTime(), (idx == 0 ? L" (profile)" : L" (histogram)" ) );

                    std::shared_ptr< adcontrols::MassSpectrum > centroid;
                    std::shared_ptr< adcontrols::MSPeakInfo > pkinfo;
                    std::unique_ptr< adcontrols::Targeting > targeting;

                    if ( auto pkd = adprocessor::dataprocessor::doCentroid( *ms, pm ) ) {
                        std::tie( pkinfo, centroid ) = *pkd;
                        centroid->addDescription( adcontrols::description( L"process", L"Centroid" ) );

                        if ( auto tm = pm.find< adcontrols::TargetingMethod >() ) {
                            targeting = std::make_unique< adcontrols::Targeting > ( *tm );
                            if ( targeting->force_find( *centroid, pk.formula(), chr->protocol() ) ) {
                                if ( target_result_finder::find( *targeting, pk.formula(), chr->protocol(), *centroid, ptree ) )
                                    annotation::add( *centroid, ptree.get_optional< int >("targeting.idx").get(), chr->protocol(), pk.formula() );
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
        auto index = ptree.get_optional< int32_t >( "targeting.idx" );
        writer->insert_reference( dataGuid, msGuid, index ? index.get() : (-1), chr->protocol() );
    }
}
