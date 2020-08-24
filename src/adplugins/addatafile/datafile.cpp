// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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

#include "datafile.hpp"
#include <adutils/cpio.hpp>
#include "rawdata_v2.hpp"
#include "rawdata_v3.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datapublisher.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <adportable/string.hpp>
#include <adlog/logger.hpp>
#include <acewrapper/input_buffer.hpp>
#include <adfs/adfs.hpp>
#include <adfs/attributes.hpp>
#include <adfs/sqlite.hpp>
#include <adfs/cpio.hpp>
#include <adutils/fsio.hpp>
#include <algorithm>
#include <iostream>
#include <memory>

/////////////////
namespace addatafile { namespace detail {

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

            void static attributes( adfs::attributes& d, const portfolio::attributes_type& s ) {
                for ( const portfolio::attribute_type& a: s )
                    d.setAttribute( a.first, a.second );
            }
        };

        struct attachment {
            static bool save( adfs::file& parent, const boost::filesystem::path& path, const portfolio::Folium& folium ) {
                boost::filesystem::path filename = path / folium.id();
                boost::any any = static_cast<const boost::any&>( folium );
                if ( ! any.empty() ) {
                    adfs::file dbThis = parent.addAttachment( folium.id() );
                    import::attributes( dbThis, folium.attributes() );
                    try {
                        adutils::cpio::save( dbThis, any );
                    } catch ( boost::bad_any_cast& ex ) {
                        assert( 0 );
                        return false;
                    }
                    for ( const portfolio::Folium& att: folium.attachments() ) {
                        boost::any any = static_cast<const boost::any&>( folium );
                        if ( !any.empty() )
                            save( dbThis, filename, att );
                    }
                    return true;
                }
                return false;
            }

            static bool save_as( adfs::file&, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
            static bool load( portfolio::Folium dst, const adfs::file& src ) {
                import::attributes( dst, src );
                for ( const adfs::file& att: src.attachments() )
                    attachment::load( dst.addAttachment( att.name() ), att );
                return true;
            }
        };

        struct folium {
            static bool save( adfs::folder& folder, const boost::filesystem::path& path, const portfolio::Folium& folium ) {
                boost::filesystem::path filename = path / folium.id();

                boost::any any = static_cast<const boost::any&>( folium );
                if ( any.empty() )
                    return false;

                if ( folder && !any.empty() ) {
                    if ( adfs::file dbf = folder.addFile( folium.id() ) ) {

                        import::attributes( dbf, folium.attributes() );
                        try {
                            adutils::cpio::save( dbf, any );
                        } catch ( boost::exception& ex ) {
                            ADTRACE() << boost::diagnostic_information( ex );
                        }

                        for ( const portfolio::Folium& att : folium.attachments() ) {
                            try {
                                detail::attachment::save( dbf, filename, att );
                            } catch ( boost::exception& ex) {
                                ADTRACE() << boost::diagnostic_information( ex );
                            }
                        }
                    }
                    return true;
                } else {
                    return false;
                }
            }

            static bool save_as( adfs::folder&, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
            static bool load( portfolio::Folium dst, const adfs::file& src ) {
                import::attributes( dst, src );
                for ( const adfs::file& att: src.attachments() )
                    attachment::load( dst.addAttachment( att.name() ), att );
                return true;
            }
        };

        struct folder {
            static bool save( adfs::filesystem& dbf, const boost::filesystem::path& path, const portfolio::Folder& folder )  {
                boost::filesystem::path pathname = path / folder.name();

                adfs::folder dbThis = dbf.addFolder( pathname.wstring() );
                import::attributes( dbThis, folder.attributes() );

                // save all files in this folder
                for ( const portfolio::Folium& folium: folder.folio() ) {
                    boost::any any = static_cast<const boost::any&>( folium );
                    if ( !any.empty() )
                        folium::save( dbThis, pathname, folium );
                }
                // recursive call for sub folders
                for ( const portfolio::Folder& subfolder: folder.folders() )
                    folder::save( dbf, pathname, subfolder );
                return true;
            }

            static bool save_as( adfs::filesystem& db, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folder& );
            static bool load( portfolio::Folder parent, const adfs::folder& adfolder ) {
                for ( const adfs::file& file: adfolder.files() )
                    folium::load( parent.addFolium( file.name() ), file );
                return true;
            }
        };


        template< class T > struct serializer {

            static std::shared_ptr< T > deserialize( const std::vector<char>& obuf ) { //adfs::detail::cpio& obuf ) {
                std::shared_ptr<T> ptr = std::make_shared<T>();
                try {
                    adfs::cpio::deserialize( *ptr, obuf.data(), obuf.size() );
                } catch ( std::exception& e ) {
                    ADERROR() << "exception: " << e.what() << " while deserializing " << typeid(T).name();
                    ptr.reset();
                }
                return ptr;
            }
        };

        struct is_valid_rawdata : public boost::static_visitor < bool > {
            template<typename T> bool operator()( T& t ) const { return t.get() != nullptr; }
        };

        struct subscribe_rawdata : boost::static_visitor < void > {
            adcontrols::dataSubscriber& subscriber_;
            subscribe_rawdata( adcontrols::dataSubscriber& t ) : subscriber_( t ) {}
            template<typename T> void operator()( T& t ) const {
                t->loadAcquiredConf();
                subscriber_.subscribe( *t );
                t->loadCalibrations();
                t->loadMSFractuation();
            }
        };

        struct undefined_spectrometers : boost::static_visitor< std::vector< std::string > > {
            template<typename T> const std::vector< std::string > operator()( T& t ) const {
                return t->undefined_spectrometers();
            }
        };

        struct undefined_data_readers : boost::static_visitor< std::vector< std::pair< std::string, boost::uuids::uuid > > > {
            template<typename T> const std::vector< std::pair< std::string, boost::uuids::uuid > > operator()( T& t ) const {
                return t->undefined_data_readers();
            }
        };

        struct apply_calibration : boost::static_visitor< bool > {
            const std::wstring& dataInterpreterClsid;
            const adcontrols::MSCalibrateResult& result;
            apply_calibration( const std::wstring& clsid, const adcontrols::MSCalibrateResult& r ) : dataInterpreterClsid( clsid ), result( r ) {
            }
            template< typename T > bool operator()( T& rawdata ) const {
                return rawdata->applyCalibration( dataInterpreterClsid, result );
            }
        };

        template<> bool apply_calibration::operator()( std::unique_ptr< addatafile::v3::rawdata >& rawdata ) const {
            return rawdata->applyCalibration( result );
        }

    }

}

using namespace addatafile;

datafile::~datafile()
{
}

datafile::datafile() : mounted_( false )
{
    //, rawdata_( new rawdata( dbf_, *this ) )
}

void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    if ( mounted_ ) {
        // handle acquired raw data
        if ( adutils::AcquiredConf::formatVersion( dbf_.db() ) == adutils::format_v2 ) {
            rawdata_ = std::make_unique< v2::rawdata >( dbf_, *this );
        } else if ( adutils::AcquiredConf::formatVersion( dbf_.db() ) == adutils::format_v3 ) {
            rawdata_ = std::make_unique< v3::rawdata >( dbf_, *this );
        }

        if ( boost::apply_visitor( detail::is_valid_rawdata(), rawdata_ ) ) {

            boost::apply_visitor( detail::subscribe_rawdata( sub ), rawdata_ );

            boost::property_tree::ptree ptop;
            auto undefined_dataReaders = boost::apply_visitor( detail::undefined_data_readers(), rawdata_ );
            if ( ! undefined_dataReaders.empty() ) {
                boost::property_tree::ptree pt;
                std::for_each( undefined_dataReaders.begin(), undefined_dataReaders.end()
                               , [&](auto& a){
                                     boost::property_tree::ptree item;
                                     item.put( "objtext", a.first );
                                     item.put( "objid", a.second );
                                     pt.push_back( std::make_pair("", item ) );
                                 });
                ptop.add_child( "dataReader", pt );
            }

            auto undefined_spectrometers = boost::apply_visitor( detail::undefined_spectrometers(), rawdata_ );
            if ( ! undefined_spectrometers.empty() ) {
                boost::property_tree::ptree pt;
                std::for_each( undefined_spectrometers.begin(), undefined_spectrometers.end()
                               , [&](auto& a){
                                     boost::property_tree::ptree item;
                                     item.put( "spectrometer", a );
                                     pt.push_back( std::make_pair( "", item ) );
                                 });
                ptop.add_child( "spectrometer", pt );
            }

            if ( !ptop.empty() ) {
                std::ostringstream json;
                boost::property_tree::write_json( json, ptop );
                sub.notify( adcontrols::dataSubscriber::idUndefinedSpectrometers, json.str() );
            }
        }

        // publish processed dataset
        portfolio::Portfolio portfolio;
        if ( loadContents( portfolio, L"/Processed" ) && processedDataset_ ) {
            processedDataset_->xml( portfolio.xml() );
            sub.subscribe( *processedDataset_ );
        } else {
            portfolio.create_with_fullpath( filename_ );
            portfolio.addFolder( L"Chromatograms" );
            portfolio.addFolder( L"Spectra" );
            processedDataset_->xml( portfolio.xml() );
            sub.subscribe( *processedDataset_ );
        }
    }
}


int
datafile::dataformat_version() const
{
    // 0 = v2, 1 = v3
    return rawdata_.which() + 2;
}

bool
datafile::open( const std::wstring& filename, bool /* readonly */ )
{
    filename_ = filename;
    processedDataset_.reset( new adcontrols::ProcessedDataset );

    if ( ( mounted_ = dbf_.mount( filename.c_str() ) ) )
        return true;

    if ( ( mounted_ = dbf_.create( filename.c_str() ) ) )
        return true;

    return false;
}

boost::any
datafile::fetch( const std::wstring& dataId, const std::wstring& dataType ) const
{
    adfs::stmt sql( dbf_.db() );
    sql.prepare( "SELECT rowid FROM file WHERE fileid = (SELECT rowid FROM directory WHERE name = ?)" );
    sql.bind( 1 ) = dataId;

#if defined DEBUG && 0
    adportable::debug(__FILE__, __LINE__) << "==> fetch(" << dataId << ", " << dataType << ")";
#endif

    boost::any any;
    size_t n = 0;
    while ( sql.step() == adfs::sqlite_row ) {
        ++n;
        assert( n <= 1 );  // should only be one results

        boost::int64_t rowid = sql.get_column_value<int64_t>( 0 );  // file id

        adfs::blob blob;
        if ( rowid && blob.open( dbf_.db(), "main", "file", "data", rowid, adfs::readonly ) ) {
            if ( blob.size() ) {
                std::vector< char > obuf( blob.size() );
                if ( blob.read( reinterpret_cast<int8_t *>(obuf.data()), blob.size() ) ) {

					if ( dataType == adcontrols::MassSpectrum::dataClass() ) {

                        any = detail::serializer< adcontrols::MassSpectrum >::deserialize( obuf );

					} else if ( dataType == adcontrols::Chromatogram::dataClass() ) {

                        any = detail::serializer< adcontrols::Chromatogram >::deserialize( obuf );

					} else if ( dataType == adcontrols::PeakResult::dataClass() ) {

						any = detail::serializer< adcontrols::PeakResult >::deserialize( obuf );

					} else if ( dataType == adcontrols::ProcessMethod::dataClass() ) {

                        any = detail::serializer< adcontrols::ProcessMethod >::deserialize( obuf );

					} else if ( dataType == adcontrols::MSCalibrateResult::dataClass() ) {

                        any = detail::serializer< adcontrols::MSCalibrateResult >::deserialize( obuf );

					} else if ( dataType == adcontrols::MSPeakInfo::dataClass() ) {

                        any = detail::serializer< adcontrols::MSPeakInfo >::deserialize( obuf );

					} else if ( dataType == adcontrols::MassSpectra::dataClass() ) {

                        any = detail::serializer< adcontrols::MassSpectra >::deserialize( obuf );

					} else if ( dataType == adcontrols::Targeting::dataClass() ) {

                        any = detail::serializer< adcontrols::Targeting >::deserialize( obuf );

					} else if ( dataType == adcontrols::QuanSample::dataClass() ) {

                        any = detail::serializer< adcontrols::QuanSample >::deserialize( obuf );

					} else if ( dataType == adcontrols::QuanSequence::dataClass() ) {

                        any = detail::serializer< adcontrols::QuanSequence >::deserialize( obuf );


                    } else {
                        ADERROR() << "Error: unknown data type in datafile::fetch(" << dataId << ", " << dataType << ")";
                        BOOST_THROW_EXCEPTION( std::bad_typeid() );
                    }
                }
            }
        }
    }
    return any;
}

////////////////////////////////////////////////////
// SaveFileAs come in here
bool
datafile::saveContents( const std::wstring& path, const portfolio::Portfolio& portfolio, const adcontrols::datafile& source )
{
    if ( ! mounted_ )
        return false;

    std::vector< std::string > removed;
    if ( portfolio.removed_dataids( removed ) > 0 ) {
        removeContents( removed );
    }

    if ( calibration_modified_ ) {
        for ( auto calib: calibrations_ )
            adutils::fsio::save_mscalibfile( dbf_, *calib.second );
    }

    adfs::stmt sql( dbf_.db() );
    sql.begin();

    dbf_.addFolder( path );

    boost::filesystem::path name( path );

    for ( const portfolio::Folder& folder : portfolio.folders() ) {
        detail::folder::save_as( dbf_, name, source, folder );
    }

    sql.commit();
    return true;
}


// Save comes here. Thus no file name pass to here
bool
datafile::saveContents( const std::wstring& path, const portfolio::Portfolio& portfolio )
{
    if ( ! mounted_ )
        return false;

    std::vector< std::string > removed;
    if ( portfolio.removed_dataids( removed ) > 0 )
        removeContents( removed );

    adfs::stmt sql( dbf_.db() );

    sql.begin();

    dbf_.addFolder( path );

    boost::filesystem::path name( path );

    for ( const portfolio::Folder& folder: portfolio.folders() )
        detail::folder::save( dbf_, name, folder );

    sql.commit();
    return true;
}

bool
datafile::loadContents( const std::wstring& path, const std::wstring& id, adcontrols::dataSubscriber& sub )
{
    if ( ! mounted_ )
        return false;
    adfs::folder folder = dbf_.findFolder( path );
    if ( folder ) {
        const std::wstring& name = folder.name();
		(void)name;
		adfs::file file = dbf_.findFile( folder, id );
        if ( file )
			sub.onFileAdded( path, file );
    }
    return false;
}

bool
datafile::loadContents( portfolio::Portfolio& portfolio, const std::wstring& query )
{
    if ( ! mounted_ )
        return false;

    portfolio.create_with_fullpath( filename_ );
    adfs::folder processed = dbf_.findFolder( query );  // L"/Processed"
    if ( ! processed )
        return false;

    // top folder should be L"Spectra" | L"Chromatograms"
    for ( const adfs::folder& folder: processed.folders() ) {
        const std::wstring& name = folder.name();
        portfolio::Folder xmlfolder = portfolio.addFolder( name );
        detail::folder::load( xmlfolder, folder );
    }

    processedDataset_ = std::make_unique< adcontrols::ProcessedDataset >();
    std::string xml = portfolio.xml();
    processedDataset_->xml( xml );

    return true;
}

bool
datafile::removeContents( const std::vector< std::string >& dataids )
{
    // not completed yet
    if ( ! mounted_ )
        return false;

    adfs::stmt sql( dbf_.db() );

    for ( auto& dataid : dataids ) {
        sql.prepare( "DELETE FROM file WHERE fileid = (SELECT fileid FROM directory WHERE name = ?)" );
        sql.bind( 1 ) = dataid;
        if ( sql.step() == adfs::sqlite_done ) {
            sql.prepare( "DELETE FROM directory WHERE name = ?" );
            sql.bind( 1 ) = dataid;
            sql.step();
        }
    }
    return true;
}

bool
datafile::applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& result )
{
    return boost::apply_visitor( detail::apply_calibration( dataInterpreterClsid, result ), rawdata_ );
}

bool
datafile::readCalibration( size_t idx, adcontrols::MSCalibrateResult& result ) const
{
    return false; //rawdata_->readCalibration( idx, result );
}

///-------------------------------------------------------------------------------------

namespace addatafile {
    namespace detail {

        bool
        attachment::save_as( adfs::file& parent, const boost::filesystem::path& path
                          , const adcontrols::datafile& source, const portfolio::Folium& folium )
        {
            boost::filesystem::path filename = path / folium.id();

            adfs::file dbThis = parent.addAttachment( folium.id() );
            import::attributes( dbThis, folium.attributes() );

            boost::any any = static_cast<const boost::any&>( folium );
            if ( any.empty() )
                any = source.fetch( folium.id(), folium.dataClass() );

            if ( ! any.empty() ) {
                try {
                    adutils::cpio::save( dbThis, any );
                } catch ( boost::bad_any_cast& ) {
                    assert( 0 );
                    return false;
                }

                for ( const portfolio::Folium& att: folium.attachments() ) {
                    save_as( dbThis, filename, source, att );
                }
            }
            return true;
        }

        //------------
        bool
        folium::save_as( adfs::folder& folder, const boost::filesystem::path& path
                         , const adcontrols::datafile& source, const portfolio::Folium& folium )
        {
            boost::filesystem::path filename = path / folium.id();

            boost::any any = static_cast<const boost::any&>( folium );
            if ( any.empty() )
                any = source.fetch( folium.id(), folium.dataClass() );

            if ( folder && !any.empty() ) {
                if ( adfs::file dbf = folder.addFile( folium.id() ) ) {

                    import::attributes( dbf, folium.attributes() );
                    try {
                        adutils::cpio::save( dbf, any );
                    } catch ( boost::exception& ex ) {
                        ADTRACE() << boost::diagnostic_information( ex );
                    }

                    for ( const portfolio::Folium& att : folium.attachments() ) {
                        boost::any any = static_cast<const boost::any&>( att );
                        try {
                            detail::attachment::save( dbf, filename, att );
                        } catch ( boost::exception& ex) {
                            ADTRACE() << boost::diagnostic_information( ex );
                        }
                    }
                }
                return true;
            } else {
                return false;
            }
        }

        //------------------------------------
        bool
        folder::save_as( adfs::filesystem& dbf, const boost::filesystem::path& path
                      , const adcontrols::datafile& source, const portfolio::Folder& folder )
        {
            boost::filesystem::path pathname = path / folder.name();

            adfs::folder dbThis = dbf.addFolder( pathname.wstring() );
            import::attributes( dbThis, folder.attributes() );

            // save all files in this folder
            for ( const portfolio::Folium& folium: folder.folio() ) {
                boost::any any = static_cast<const boost::any&>( folium );
                if ( any.empty() )
                    any = source.fetch( folium.id(), folium.dataClass() );
                if ( !any.empty() )
                    folium::save_as( dbThis, pathname, source, folium );
            }

            // recursive save sub folders
            for ( const portfolio::Folder& subfolder: folder.folders() )
                folder::save_as( dbf, pathname, source, subfolder );

            return true;
        }

    }
}

//////////////////////////
