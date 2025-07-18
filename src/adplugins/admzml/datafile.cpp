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

#include "chromatogram.hpp"
#include "datafile.hpp"
#include "export_to_adfs.hpp"
#include "mzml.hpp"
#include "mzmlreader.hpp"
#include "mzmlspectrum.hpp"
#include "mzmlwalker.hpp"
#include "serializer.hpp"
#include <adcontrols/countinghistogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/datainterpreterbroker.hpp>
#include <adcontrols/datapublisher.hpp>
// #include <adcontrols/datareader.hpp>
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
#include <pugixml.hpp>

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

namespace mzml {

    // ------------> from addatafile::datafile.cpp
    struct import {
        //---
        void static attributes( portfolio::Folium& d, const adfs::attributes& s )   {
            for ( adfs::attributes::vector_type::const_iterator it = s.begin(); it != s.end(); ++it )
                    d.setAttribute( it->first, it->second );
        }

        void static attributes( portfolio::Folder& d, const adfs::attributes& s )  {
            for ( adfs::attributes::vector_type::const_iterator it = s.begin(); it != s.end(); ++it )
                d.setAttribute( it->first, it->second );
        }

        void static attributes( adfs::attributes& d, const std::vector< std::pair< std::wstring, std::wstring > >& s ) {
            for ( const auto& a: s )
                d.setAttribute( a.first, a.second );
        }
    };

    struct attachment {
        static bool load( portfolio::Folium dst, const adfs::file& src ) {
            import::attributes( dst, src );
            for ( const adfs::file& att: src.attachments() )
                attachment::load( dst.addAttachment( att.name() ), att );
            return true;
        }
    };

    struct folium {
        static bool load( portfolio::Folium dst, const adfs::file& src ) {
            import::attributes( dst, src );
            for ( const adfs::file& att: src.attachments() )
                attachment::load( dst.addAttachment( att.name() ), att );
            return true;
        }
    };

    struct folder {
        static bool load( portfolio::Folder parent, const adfs::folder& adfolder ) {
            for ( const adfs::file& file: adfolder.files() )
                folium::load( parent.addFolium( file.name() ), file );
            return true;
        }
    };
    // <----------------------


    class datafile::impl {
    public:
        std::shared_ptr< mzML > mzml_;
		std::unique_ptr< adcontrols::ProcessedDataset > processedDataset_;
        std::map< std::string, std::shared_ptr< adcontrols::Chromatogram > > vChro_;
        std::map< std::string, std::shared_ptr< adcontrols::MassSpectrum > > vSpectrum_;
        boost::json::object json_;
        std::filesystem::path filepath_;
        adfs::filesystem dbf_;
        bool mounted_;

        impl() : mzml_( std::make_shared< mzML >() )
               , processedDataset_( std::make_unique< adcontrols::ProcessedDataset >() )
               , mounted_( false )
            {
        }

        bool
        loadContents( portfolio::Portfolio& portfolio, const std::wstring& query ) {
            if ( ! mounted_ )
                return false;
            portfolio.create_with_fullpath( filepath_.wstring() );
            adfs::folder processed = dbf_.findFolder( query );  // L"/Processed"
            if ( ! processed ) {
                return false;
            }
            // top folder should be L"Spectra" | L"Chromatograms"
            for ( const adfs::folder& folder: processed.folders() ) {
                const std::wstring& name = folder.name();
                portfolio::Folder xmlfolder = portfolio.addFolder( name );
                folder::load( xmlfolder, folder );
            }
            processedDataset_ = std::make_unique< adcontrols::ProcessedDataset >();
            std::string xml = portfolio.xml();
            processedDataset_->xml( xml );
            return true;
        }
    };

}

using namespace mzml;

datafile::~datafile()
{
}

datafile::datafile() : impl_( std::make_unique< impl >() )
{
}

void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    ADDEBUG() << "################### " << __FUNCTION__ << " ################";

    sub.subscribe( *impl_->mzml_ ); // LCMSDataset

    if ( impl_->processedDataset_ )
        sub.subscribe( *impl_->processedDataset_ );
#if 0
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
#endif
}

bool
datafile::open( const std::filesystem::path& path, bool /* readonly */ )
{
    impl_->filepath_ = path;

    ADDEBUG() << "datafile::open(" << path << ")";

    impl_->mzml_ = std::make_unique< mzML >();
    portfolio::Portfolio portfolio;

    if ( path.extension() == ".adfs" ) {
        if (( impl_->mounted_ = impl_->dbf_.mount( path  ) )) {
            impl_->loadContents( portfolio, L"/Processed" );
        }
    } else if ( impl_->mzml_->open( path ) ) {
        portfolio.create_with_fullpath( path.wstring() );
        auto folder= portfolio.addFolder( L"Chromatograms" );
        for ( auto chro: impl_->mzml_->import_chromatograms() ) {
            std::string name = chro->display_name() ? *chro->display_name() : chro->make_title();
            auto folium = folder.addFolium( name ).assign( chro, chro->dataClass() );
            impl_->vChro_.emplace( folium.id<char>(), chro );
        }
        impl_->processedDataset_->xml( portfolio.xml() );
    }
    return true;
}

int
datafile::dataformat_version() const
{
    return 3;
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

bool
datafile::export_rawdata( const adcontrols::datafile& db ) const
{
    ADDEBUG() << __FUNCTION__;

    auto sqlite = db.sqlite();
    export_to_adfs exporter( std::move( sqlite ) );
    exporter( *impl_->mzml_ );

    return true;
}
