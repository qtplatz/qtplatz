/**************************************************************************
** Copyright (C) 2020-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanexportprocessor.hpp"
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
#include <adcontrols/msfinder.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/moltable.hpp>
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
#include <adcontrols/quanresponsemethod.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adcontrols/quan/extract_by_mols.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/file.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adportable/json_helper.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adprocessor/mschromatogramextractor.hpp>
#include <adutils/cpio.hpp>
#include <adwidgets/progressinterface.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <filesystem>
#include <set>

namespace quan {

    struct save_chromatogram {

        static std::wstring make_title( const wchar_t * dataSource, const adcontrols::Chromatogram& c ) {
            std::filesystem::path path( dataSource );

            auto extract_by_mols = boost::json::value_to< adcontrols::quan::extract_by_mols >(
                adportable::json_helper::find( c.generatorProperty(), "generator.extract_by_mols" ) );

            auto wform = adportable::utf::to_wstring( extract_by_mols.wform_type );
            const auto& mol = extract_by_mols.moltable_;

            return ( boost::wformat( L"%s #%d W(%.1fmDa) {%s}-%s" )
                     % adportable::utf::to_wstring( mol.formula )
                     % mol.protocol
                     % ( mol.width * 1000 )
                     % path.stem().wstring()
                     % wform ).str();
        }

        static boost::uuids::uuid
        save( std::shared_ptr< QuanDataWriter > writer
              , const wchar_t * dataSource
              , std::shared_ptr< adcontrols::Chromatogram > chromatogram
              , const adcontrols::ProcessMethod& procm, size_t idx )   {

            auto title = make_title( dataSource, *chromatogram );
            if ( adfs::file file = writer->write( *chromatogram, title ) ) {
                auto fGuid = boost::uuids::string_generator()( file.name() );
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

QuanExportProcessor::~QuanExportProcessor()
{
}

QuanExportProcessor::QuanExportProcessor( QuanProcessor * processor
                                          , std::vector< adcontrols::QuanSample >& samples
                                          , std::shared_ptr< adwidgets::ProgressInterface > p )
    : procm_( std::make_shared< adcontrols::ProcessMethod >( *processor->procmethod() ) ) // deep copy
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
        for ( const auto& sample: samples ) {
            if ( auto dp = std::make_shared< adprocessor::dataprocessor >() ) {
                std::string errmsg;
                if ( dp->open( std::filesystem::path( sample.dataSource() ), errmsg ) ) {
                    adfs::stmt sql( *(dp->db()) );
                    if ( sql.prepare( "SELECT COUNT(*) as C FROM AcquiredData GROUP BY fcn ORDER BY C DESC" ) ) {
                        if ( sql.step() == adfs::sqlite_row ) {
                            n_spectra += sql.get_column_value< uint64_t >( 0 );
                            samples_.emplace_back( sample ); // add to process list
                        }
                    }
                } else {
                    ADDEBUG() << "-- db open error: " << sample.dataSource() << "\n\terror: " << errmsg;
                }
            }
        }

        progress_current_ = 0;
        progress_total_ = samples_.size();
        n_spectra *= 2; // PKD, AVG
        progress_total_ =
            ( n_spectra > std::numeric_limits< decltype( progress_total_ ) >::max() ) ? std::numeric_limits< decltype( progress_total_ ) >::max() : n_spectra;
    }
    // <-- end dry run
    (*progress_)( int( progress_current_ ), int( progress_total_) );
}

QuanProcessor *
QuanExportProcessor::processor()
{
    return processor_.get();
}


bool
QuanExportProcessor::operator()( std::shared_ptr< QuanDataWriter > writer )
{
    auto cm = procm_->find< adcontrols::CentroidMethod >();
    auto qm = procm_->find< adcontrols::QuanMethod >();

    if ( !cm || !qm )
        return false;

    // adcontrols::QuanCompounds compounds;
    // if ( auto qc = procm_->find< adcontrols::QuanCompounds >() )
    //     compounds = *qc;

    int32_t proto = 0; // ( protocols.size() == 1 ) ? *protocols.begin() : (-1);

    for ( auto& sample : samples_ ) {

        const std::filesystem::path stem = std::filesystem::path( sample.dataSource() ).stem();
        auto dp = std::make_shared< adprocessor::dataprocessor >();
        std::string emsg;

        if ( dp->open( std::filesystem::path( sample.dataSource() ), emsg ) ) {
            if ( auto raw = dp->rawdata() ) {
                if ( raw->dataformat_version() < 3 )
                    return false;

                std::array< std::shared_ptr< const adcontrols::DataReader >, 2 > readers {{nullptr, nullptr}};
                for ( auto reader: raw->dataReaders() ) {
                    if ( reader->objtext() == "1.u5303a.ms-cheminfo.com" )     // u5303a AVG
                        readers[ 0 ] = reader;
                    if ( reader->objtext() == "pkd.1.u5303a.ms-cheminfo.com" ) // u5303a PKD
                        readers[ 1 ] = reader;
                }

                size_t idx = 0;
                for ( auto reader: readers ) {
                    adcontrols::ProcessMethod pm( *procm_ );
                    if ( reader ) {
                        auto spectra = dp->createSpectrogram( procm_
                                                              , reader.get()
                                                              , proto
                                                              , [&]( size_t n, size_t total ){
                                                                    (*progress_)();
                                                                    return false;
                                                                });
                        using adportable::utf;
                        auto title = ( boost::wformat( L"%s-%s-p%d" ) % stem.wstring() % (idx == 0 ? L"AVG" : L"PKD") % 0 ).str();
                        if ( auto file = writer->write( *spectra, title ) ) {
                            auto fid = boost::uuids::string_generator()( file.name() );
                            writer->insert_spectrogram( fid, *spectra, *dp, idx );
                        }
                    }
                    ++idx;
                }
            }
        }
        writer->insert_table( sample );
        (*progress_)();
        processor_->complete( &sample );
    }
    document::instance()->sample_processed( this );
    return true;
}
