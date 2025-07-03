// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

// #if __APPLE__ && (__GNUC_LIBSTD__ <= 4) && (__GNUC_LIBSTD_MINOR__ <= 2)
// #  define BOOST_NO_CXX11_RVALUE_REFERENCES
// #endif

#include "andichromatography.hpp"
#include "andims.hpp"
#include "chromatogram.hpp"
#include "datafile.hpp"
#include "ncfile.hpp"
#include <netcdf.h>
#include <adcontrols/countinghistogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/datainterpreterbroker.hpp>
#include <adcontrols/datapublisher.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/textfile.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adlog/logger.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/uuid/uuid.hpp>
#include <filesystem>
#include <optional>
#include <boost/json.hpp>

// ---------------------- overloads -------------------->
namespace {
    // helper type for the visitor #4
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };

    // explicit deduction guide (not needed as of C++20)
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
}
//<---------------------- overloads --------------------

namespace adnetcdf {

    class datafile::impl {
    public:
		std::unique_ptr< adcontrols::ProcessedDataset > processedDataset_;
        double accelVoltage_;
        double length_;
        double tDelay_;
        std::string model_;
        std::map< std::string, std::shared_ptr< adcontrols::Chromatogram > > vChro_;
        std::map< std::string, std::shared_ptr< adcontrols::MassSpectrum > > vSpectrum_;
        std::variant< std::shared_ptr< AndiChromatography >
                      , std::shared_ptr< AndiMS > > cdf_;
        boost::json::object json_;

        impl() : processedDataset_( std::make_unique< adcontrols::ProcessedDataset >() )
               , accelVoltage_( 0 )
               , length_( 0 )
               , tDelay_( 0 ) {
        }
    };

}

using namespace adnetcdf;

datafile::~datafile()
{
}

datafile::datafile() : impl_( std::make_unique< impl >() )
{
}

void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    // subscribe acquired dataset <LCMSDataset>
    // No LC/GC data supported
    // sub.subscribe( *this );
    // subscribe processed dataset
    std::visit( overloaded{
            [&]( const auto& arg ) {}
                , [&]( const std::shared_ptr< AndiMS >& p ){
                    sub.subscribe( *p );
                }
                }, impl_->cdf_ );

    if ( impl_->processedDataset_ )
        sub.subscribe( *impl_->processedDataset_ );

    if ( auto db = sub.db() ) {

        if ( ! impl_->model_.empty() ) { // has scan law
            auto models = adcontrols::MassSpectrometer::installed_models();
            auto it = std::find_if( models.begin(), models.end()
                                    , [&]( const std::pair< boost::uuids::uuid, std::string >& t ){
                return t.second == impl_->model_;
            });
            if ( it == models.end() )
                return; // can't find clsid

            static std::string misc_spectrometer = "9d7afc9f-f4c8-4a2d-a462-23aafde8d99f";

            adfs::stmt sql( *db );
            sql.prepare( "INSERT OR REPLACE INTO ScanLaw ("
                         " objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer)"
                         " VALUES ( ?,?,?,?,?,? )" );
            sql.bind( 1 ) = boost::uuids::uuid{ 0 };           // master observer
            sql.bind( 2 ) = std::string( "master.observer" );  // signal observer text name
            sql.bind( 3 ) = impl_->accelVoltage_;
            sql.bind( 4 ) = impl_->tDelay_;
            sql.bind( 5 ) = misc_spectrometer; // ScanLaw.spectrometer = Spectrometer.id
            sql.bind( 6 ) = it->first;  // uuid_massspectrometer;

            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << "sqlite error";

            sql.prepare( "INSERT OR REPLACE INTO Spectrometer ( id, scanType, description, fLength ) VALUES ( ?,?,?,? )" );
            sql.bind( 1 ) = misc_spectrometer;  // ScanLaw.spectrometer = Spectrometer.id
            sql.bind( 2 ) = 0;
            sql.bind( 3 ) = std::string( "CSV Imported" );
            sql.bind( 4 ) = impl_->length_;

            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << "sqlite error";
        }
    }
}

bool
datafile::open( const std::filesystem::path& filename, bool /* readonly */ )
{
    portfolio::Portfolio portfolio;
    portfolio.create_with_fullpath( filename );

    if ( auto file = adnetcdf::netcdf::open( filename ) ) {

        auto folder = portfolio.addFolder( L"Chromatograms" );

        if ( auto temp = file.get_att_text( "aia_template_revision" ) ) {
            auto andi = std::make_shared< AndiChromatography >();
            for ( auto chro: andi->import( file ) ) {
                auto folium = folder.addFolium( chro->make_title() ).assign( chro, chro->dataClass() );
                impl_->vChro_.emplace( folium.id<char>(), chro );
            }
            impl_->cdf_ = std::move( andi );
        } else if ( auto temp = file.get_att_text( "ms_template_revision" ) ) {
            auto andi = std::make_shared< AndiMS >();

            for ( auto& chro: andi->import( file ) ) {
                auto folium = folder.addFolium( chro->make_title() ).assign( chro, chro->dataClass() );
                impl_->vChro_.emplace( folium.id<char>(), chro );
            }
            impl_->cdf_ = std::move( andi );
        }

        // has spectra ?
        std::visit( overloaded{
                [&]( const auto& arg ) { /* do nothing */ }
                    , [&]( const std::shared_ptr< AndiMS >& p ) {
                        if ( p->has_spectra() ) {
                            auto folder = portfolio.addFolder( L"Spectra" );
                            auto v = p->dataReaders();
                            if ( ! v.empty() ) {
                                if ( auto reader = v.at(0) ) {
                                    if ( auto ms = reader->coaddSpectrum( reader->begin(), reader->end() ) ) {
                                        auto folium = folder.addFolium( "Coadded spectrum" ).assign( ms, ms->dataClass() );
                                        impl_->vSpectrum_.emplace( folium.id<char>(), ms );
                                    }
                                }
                            }
                        }
                    }
                    }, impl_->cdf_ );

        impl_->processedDataset_->xml( portfolio.xml() );
        return true;
    }
    return false;
}


boost::any
datafile::fetch( const std::string& path, const std::string& dataType ) const
{
    if ( dataType == adportable::utf::to_utf8( adcontrols::MassSpectrum::dataClass() ) ) {
        auto it = impl_->vSpectrum_.find( path );
        if ( it != impl_->vSpectrum_.end() )
            return it->second;
    }
    if ( dataType == adportable::utf::to_utf8( adcontrols::Chromatogram::dataClass() ) ) {
        auto it = impl_->vChro_.find( path );
        if ( it != impl_->vChro_.end() )
            return it->second;
    }
    ADDEBUG() << "Error: ======== " << __FUNCTION__ << std::make_pair( path, dataType ) << " not in the object"; // guid, Chromatogram
    return {};
}

boost::any
datafile::fetch( const std::wstring& path, const std::wstring& dataType ) const
{
    return fetch( adportable::utf::as_utf8( path ), adportable::utf::as_utf8( dataType ) );
}
