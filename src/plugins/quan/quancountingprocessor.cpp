/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quancountingprocessor.hpp"
#include "findcompounds.hpp"
#include "quanprocessor.hpp"
#include "quanchromatograms.hpp"
#include "quanchromatogramprocessor.hpp"
#include "quandatawriter.hpp"
#include "document.hpp"
#include "../plugins/dataproc/dataprocconstants.hpp"
#include <coreplugin/progressmanager/progressmanager.h>
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
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
#include <adcontrols/quanresponsemethod.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/file.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adprocessor/mschromatogramextractor.hpp>
#include <adutils/cpio.hpp>
#include <adwidgets/progressinterface.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
// #include <boost/property_tree/ptree.hpp>
#include <set>

namespace quan {

    struct save_chromatogram {

        static std::wstring make_title( const wchar_t * dataSource, const adcontrols::Chromatogram& c )  {
            boost::filesystem::path path( dataSource );

            auto extract_by_mols = boost::json::value_to< adcontrols::quan::extract_by_mols >(
                adportable::json_helper::find( c.generatorProperty(), "generator.extract_by_mols" ) );

            std::string wform = extract_by_mols.wform_type;
            const auto& mol = extract_by_mols.moltable_;

            return ( boost::wformat( L"%s #%d W(%.1fmDa) {%s}-%s" )
                     % adportable::utf::to_wstring( mol.formula )
                     % mol.protocol
                     % ( mol.width * 1000 )
                     % path.stem().wstring()
                     % adportable::utf::to_wstring( wform ) ).str();

            // auto wform = pt.get_optional< std::string >( "generator.extract_by_mols.wform_type" );

            // if ( auto mol = pt.get_child_optional( "generator.extract_by_mols.moltable" ) ) {
            //     auto formula = mol.get().get_optional< std::string >( "formula" );
            //     auto width = mol.get().get_optional< double >( "width" );
            //     auto proto = mol.get().get_optional< int32_t >( "protocol" );

            //     return ( boost::wformat( L"%s #%d W(%.1fmDa) {%s}-%s" )
            //              % ( formula ? adportable::utf::to_wstring( formula.get() ) : L"" )
            //              % ( proto ? proto.get() : (-1) )
            //              % ( width ? width.get() *1000 : 0.0 )
            //              % path.stem().wstring()
            //              % ( wform ? adportable::utf::to_wstring( wform.get() ) : L"n/a" ) ).str();
            // } else {
            //     return ( boost::wformat( L"{%s}" ) % path.stem().wstring() ).str();
            // }
        }

        static boost::uuids::uuid
        save( std::shared_ptr< QuanDataWriter > writer
              , const wchar_t * dataSource
              , std::shared_ptr< adcontrols::Chromatogram > chromatogram
              , const adcontrols::ProcessMethod& procm, size_t idx )   {

            auto title = make_title( dataSource, *chromatogram );
            if ( adfs::file file = writer->write( *chromatogram, title ) ) {
                auto fGuid = boost::uuids::string_generator()( file.name() );
                // chromatogram->ptree().put( "folder.dataGuid", fGuid );
                boost::json::object jobj;
                if ( auto prop = chromatogram->generatorProperty() ) {
                    auto jv = boost::json::parse( *prop );
                    jobj = jv.as_object();
                }
                jobj[ "folder" ] = boost::json::object{{ "dataGuid", boost::uuids::to_string( fGuid ) }};
                // chromatogram->ptree().put( "folder.dataGuid", fGuid );
                chromatogram->setGeneratorProperty( boost::json::serialize( jobj ));

                return fGuid;
            }
            return {{ 0 }};
        }
    };
}

using namespace quan;

QuanCountingProcessor::~QuanCountingProcessor()
{
}

QuanCountingProcessor::QuanCountingProcessor( QuanProcessor * processor
                                              , std::vector< adcontrols::QuanSample >& samples
                                              , std::shared_ptr< adwidgets::ProgressInterface > p )
    : raw_( 0 )
    , samples_( samples )
    , procm_( std::make_shared< adcontrols::ProcessMethod >( *processor->procmethod() ) ) // deep copy
    , cformula_( std::make_shared< adcontrols::ChemicalFormula >() )
    , processor_( processor->shared_from_this() )
    , progress_( p )
    , progress_current_( 0 )
    , progress_total_( 0 )
    , cXmethods_{{ std::make_unique< adcontrols::MSChromatogramMethod >(), std::make_unique< adcontrols::MSChromatogramMethod >() }}
{
    if ( !samples.empty() )
        path_ = samples[ 0 ].dataSource();

    progress_current_ = 0;
    progress_total_ = samples.size();

    // dry run
    {
        size_t n_spectra( 0 );
        for ( const auto& sample: samples_ ) {
            std::unique_ptr< adcontrols::datafile > file( adcontrols::datafile::open( sample.dataSource(), /* read-only */ true ) );
            if ( file ) {
                struct subscriber : adcontrols::dataSubscriber {
                    const adcontrols::LCMSDataset * raw;
                    subscriber() : raw( 0 ) {}
                    bool subscribe( const adcontrols::LCMSDataset& d ) { raw = &d; return true; }
                } subscribe;
                file->accept( subscribe );
                size_t n = 0;
                if ( subscribe.raw && subscribe.raw->db() ) {
                    adfs::stmt sql( *subscribe.raw->db() );
                    sql.prepare( "SELECT COUNT(*) FROM AcquiredData GROUP BY fcn" );
                    while ( sql.step() == adfs::sqlite_row )
                        n = std::max( uint64_t( n ), sql.get_column_value< uint64_t >( 0 ) );
                    n_spectra += n;
                }
            }
        }
        progress_current_ = 0;
        progress_total_ = ( n_spectra > std::numeric_limits< decltype( progress_total_ ) >::max() ) ? std::numeric_limits< decltype( progress_total_ ) >::max() : n_spectra;
    }
    // <-- end dry run

    if ( auto pCompounds = procm_->find< adcontrols::QuanCompounds >() ) {

        // mass chromatograms extraction method
        // split molecule list into either counting or profile
        pCompounds->convert_if( cXmethods_[ 0 ]->molecules(), []( const adcontrols::QuanCompound& comp ){ return !comp.isCounting();} );
        pCompounds->convert_if( cXmethods_[ 1 ]->molecules(), []( const adcontrols::QuanCompound& comp ){ return comp.isCounting();} );

        if ( auto lkm = procm_->find< adcontrols::MSLockMethod >() ) {
#ifndef NDEBUG
            // ADDEBUG() << lkm->toJson();
#endif
            for ( auto& cm: cXmethods_ )
                cm->setLockmass( lkm->enabled() );
        }

        if ( auto targeting_method = procm_->find< adcontrols::TargetingMethod >() ) {
            for ( auto& cm: cXmethods_ )
                cm->width( targeting_method->tolerance( targeting_method->toleranceMethod() ), adcontrols::MSChromatogramMethod::widthInDa );

            auto tm = std::make_shared< adcontrols::TargetingMethod >( *targeting_method );
            pCompounds->convert( tm->molecules() );
            *procm_ *= *tm;
        }
    }

    (*progress_)( int( progress_current_ ), int( progress_total_) );
}

QuanProcessor *
QuanCountingProcessor::processor()
{
    return processor_.get();
}


bool
QuanCountingProcessor::operator()( std::shared_ptr< QuanDataWriter > writer )
{
    auto cm = procm_->find< adcontrols::CentroidMethod >();
    auto qm = procm_->find< adcontrols::QuanMethod >();
    // auto rm = procm_->find< adcontrols::QuanResponseMethod >();

    if ( !cm || !qm )
        return false;

    adcontrols::QuanCompounds compounds;
    if ( auto qc = procm_->find< adcontrols::QuanCompounds >() )
        compounds = *qc;

    std::set< int32_t > protocols;
    for ( const auto& cmpd: compounds )
        protocols.insert( cmpd.protocol() );
    int32_t proto = ( protocols.size() == 1 ) ? *protocols.begin() : (-1);

    int channels( 0 ); // 1 := counting channel use, 2 := profile channel use, 3 := both
    for ( const auto& c: compounds )
        channels |= c.isCounting() ? 1 : 2;

    double tolerance = 0.001;
    if ( auto tm = procm_->find< adcontrols::TargetingMethod >() )
        tolerance = tm->tolerance( adcontrols::idToleranceDaltons );

    auto lkMethod = procm_->find< adcontrols::MSLockMethod >();

    for ( auto& sample : samples_ ) {

        const boost::filesystem::path stem = boost::filesystem::path( sample.dataSource() ).stem();
        auto dp = std::make_shared< adprocessor::dataprocessor >();
        std::wstring emsg;

        if ( dp->open( sample.dataSource(), emsg ) ) {
            if ( auto raw = dp->rawdata() ) {
                if ( raw->dataformat_version() < 3 )
                    return false;
                std::array< std::shared_ptr< const adcontrols::DataReader >, 2 > readers {{nullptr, nullptr}};
                for ( auto reader: raw->dataReaders() ) {
                    if ( reader->objtext().find( "waveform" ) != std::string::npos && cXmethods_[0]->molecules().size() ) // profile
                        readers[ 0 ] = reader;
                    if ( reader->objtext().find( "histogram" ) != std::string::npos && cXmethods_[1]->molecules().size() ) // counting
                        readers[ 1 ] = reader;
                }
                auto extractor = std::make_unique< adprocessor::v3::MSChromatogramExtractor >( raw );
                auto pCompounds = procm_->find< adcontrols::QuanCompounds >();
                if ( !pCompounds )
                    return false;
                size_t idx = 0;
                for ( auto reader: readers ) {
                    adcontrols::ProcessMethod pm( *procm_ );
                    pm *= (*cXmethods_[ idx ]);
                    if ( reader ) {
                        std::vector< std::shared_ptr< adcontrols::Chromatogram > > clist;
                        extractor->extract_by_mols( clist, pm, reader, [&]( size_t, size_t )->bool{ return (*progress_)(); } );
                        for ( auto chro: clist ) {
                            auto dataGuid = save_chromatogram::save( writer, sample.dataSource(), chro, pm, idx );
                            writer->addCountingResponse( dataGuid, sample, *chro );
                        }
                        ADDEBUG() << "################# MSLock size: " << extractor->lkms().size();
                        writer->addMSLock( sample, extractor->lkms() );
                        writer->addMSLock( dp, extractor->lkms() );
                    }
                }
            }

            FindCompounds findCompounds( compounds, *cm, tolerance );

            if ( channels & 0x01 ) { // counting
                if ( auto hist = dp->readSpectrumFromTimeCount() )
                    findCompounds.doCentroid( dp, hist, true );
            }

            if ( channels & 0x02 ) { // profile
                if ( auto profile = dp->readCoAddedSpectrum( false, proto ) )
                    findCompounds.doCentroid( dp, profile, false );
            }

            if ( lkMethod && lkMethod->enabled() )
                findCompounds.doMSLock( *lkMethod, !( channels & 0x02 ) );

            if ( channels & 0x01 ) { // counting
                findCompounds( dp, true );
                findCompounds.write( writer, stem.wstring(), procm_, sample, true, dp );
            }

            if ( channels & 0x02 ) { // profile
                findCompounds( dp, false );
                findCompounds.write( writer, stem.wstring(), procm_, sample, false, dp );
            }
        }

        writer->insert_table( sample );
        (*progress_)();
        processor_->complete( &sample );
    }
    document::instance()->sample_processed( this );
    return true;
}

bool
QuanCountingProcessor::subscribe( const adcontrols::LCMSDataset& d )
{
    raw_ = &d;
    return true;
}

bool
QuanCountingProcessor::subscribe( const adcontrols::ProcessedDataset& d )
{
    portfolio_ = std::make_shared< portfolio::Portfolio >( d.xml() );
    return true;
}
