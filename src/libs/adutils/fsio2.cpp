/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "fsio2.hpp"
#include "cpio.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adinterface/method.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <regex>

namespace adutils { namespace detail {

    static adcontrols::datafile * nullfile(0);

        struct folder {
            static bool save( adfs::filesystem& db, const std::filesystem::path&, const adcontrols::datafile&, const portfolio::Folder& );
            static bool load( portfolio::Folder parent, const adfs::folder& adf );
        };

        struct folium {
            static bool save( adfs::folder&, const std::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
            static bool load( portfolio::Folium dst, const adfs::file& src );

            // added 2023-08-09
            static bool save( adfs::folder&, const std::filesystem::path&, const portfolio::Folium& );
        };

        struct attachment {
            static bool save( adfs::file& parent, const std::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
            static bool load( portfolio::Folium dst, const adfs::file& adf );
        };

        struct import {
            template< typename T = std::wstring > static void attributes( adfs::attributes&, const std::vector< std::pair< T, T > >& );
            static void attributes( portfolio::Folium&, const adfs::attributes& );
            static void attributes( portfolio::Folder&, const adfs::attributes& );
        };

        template<> void import::attributes< std::wstring >( adfs::attributes&, const std::vector< std::pair< std::wstring, std::wstring > >& );
        template<> void import::attributes< std::string >( adfs::attributes&, const std::vector< std::pair< std::string, std::string > >& );
}
}

using namespace adutils;

// static
bool
fsio2::saveContents( adfs::filesystem& dbf, const std::wstring& path, const portfolio::Portfolio& portfolio, const adcontrols::datafile& source )
{
    adfs::stmt sql( dbf.db() );
    sql.begin();

    dbf.addFolder( path );

    std::filesystem::path name( path ); // this should be "/Processed"

    for ( const portfolio::Folder& folder: portfolio.folders() )
        detail::folder::save( dbf, name, source, folder );

    sql.commit();

    return true;
}


///////////////////////////////

bool
detail::attachment::save( adfs::file& parent, const std::filesystem::path& path
                          , const adcontrols::datafile& source, const portfolio::Folium& folium )
{
    std::filesystem::path filename = ( path / folium.id() ).generic_wstring();

    adfs::file dbThis = parent.addAttachment( folium.id() );
    import::attributes( dbThis, folium.attributes() );

#if defined DEBUG && 0
    const std::wstring& dataclass = folium.dataClass();
    const std::wstring& name = folium.name();
    adportable::debug( __FILE__, __LINE__ ) << "addatafile::detail::attachment::save("
                                            << dataclass << ", " << name << ")";
#endif
    boost::any any = static_cast<const boost::any&>( folium );
    if ( any.empty() && (&source != nullfile ) )
        any = source.fetch( folium.id(), folium.dataClass() );

    if ( ! any.empty() ) {
        try {
            adutils::cpio::save( dbThis, any );
		} catch ( boost::exception& e ) {
			typedef boost::error_info< struct tag_errmsg, std::string > errmsg_info;
			e << errmsg_info("adutils::detail::attachement::save");
			throw;
        }

        for ( const portfolio::Folium& att: folium.attachments() )
            save( dbThis, filename, source, att );
    }
    return true;
}

bool
detail::folium::save( adfs::folder& folder, const std::filesystem::path& path
                      , const adcontrols::datafile& source, const portfolio::Folium& folium )
{
    std::filesystem::path filename = ( path / folium.id() ).generic_wstring();

    boost::any any = static_cast<const boost::any&>( folium );
    if ( any.empty() && (&source != nullfile ) )
        any = source.fetch( folium.id(), folium.dataClass() );

    if ( folder && !any.empty() ) {
        adfs::file dbf = folder.addFile( folium.id() );

        import::attributes( dbf, folium.attributes() );
        cpio::save( dbf, any );

        for ( const portfolio::Folium& att: folium.attachments() ) {
			try {
				detail::attachment::save( dbf, filename, source, att );
			} catch ( boost::exception& e ) {
				ADDEBUG() << boost::diagnostic_information( e );
			}
		}
    }
    return true;
}

bool
detail::folder::save( adfs::filesystem& dbf, const std::filesystem::path& path
                      , const adcontrols::datafile& source, const portfolio::Folder& folder )
{
    std::filesystem::path pathname = ( path / folder.name() ).generic_string();

    adfs::folder dbThis = dbf.addFolder( pathname.wstring() );
    import::attributes( dbThis, folder.attributes() );

    // save all files in this folder
    for ( const portfolio::Folium& folium: folder.folio() )
        folium::save( dbThis, pathname, source, folium );

    // recursive save sub folders
    for ( const portfolio::Folder& subfolder: folder.folders() )
        folder::save( dbf, pathname, source, subfolder );

    return true;
}

bool
detail::folder::load( portfolio::Folder parent, const adfs::folder& adfolder )
{
    for ( const adfs::file& file: adfolder.files() )
        folium::load( parent.addFolium( file.name() ), file );
    return true;
}

bool
detail::folium::load( portfolio::Folium dst, const adfs::file& src )
{
#if defined DEBUG && 0
    adportable::debug(__FILE__, __LINE__)
        << ">> folium::load(" << src.attribute(L"name") << ") "
        << src.attribute(L"dataType") << ", " << src.attribute(L"dataId");
#endif
    import::attributes( dst, src );
    for ( const adfs::file& att: src.attachments() )
        attachment::load( dst.addAttachment( att.name() ), att );
    return true;
}

bool
detail::attachment::load( portfolio::Folium dst, const adfs::file& src )
{
#if defined DEBUG && 0
    adportable::debug(__FILE__, __LINE__)
        << " +++ attachment::load(" << src.attribute(L"name") << ") "
        << src.attribute(L"dataType") << ", " << src.attribute(L"dataId");
#endif
    import::attributes( dst, src );
    for ( const adfs::file& att: src.attachments() )
        attachment::load( dst.addAttachment( att.name() ), att );
    return true;
}

//---

void
detail::import::attributes( portfolio::Folium& d, const adfs::attributes& s )
{
    for ( adfs::attributes::vector_type::const_iterator it = s.begin(); it != s.end(); ++it )
        d.setAttribute( it->first, it->second );
}

void
detail::import::attributes( portfolio::Folder& d, const adfs::attributes& s )
{
    for ( adfs::attributes::vector_type::const_iterator it = s.begin(); it != s.end(); ++it )
        d.setAttribute( it->first, it->second );
}

template<> void
detail::import::attributes( adfs::attributes& d, const std::vector< std::pair< std::wstring, std::wstring > >& s )
{
    for ( const auto& a: s )
        d.setAttribute( a.first, a.second );
}

template<> void
detail::import::attributes( adfs::attributes& d, const std::vector< std::pair< std::string, std::string > >& s )
{
    for ( const auto& a: s )
        d.setAttribute( a.first, a.second );
}

// static
bool
fsio2::appendOnFile( const std::wstring& filename, const portfolio::Folium& folium, const adcontrols::datafile& source )
{
    adfs::filesystem fs;

    if ( !std::filesystem::exists( filename ) ) {
        if ( !fs.create( filename.c_str() ) )
            return false;
    } else {
        if ( ! fs.mount( filename.c_str() ) )
            return false;
    }

	portfolio::Folder folder = folium.parentFolder();

    // "/Processed/Spectra" | "/Processed/MSCalibration"
    std::filesystem::path pathname = ( std::filesystem::path( "/Processed" ) / folder.name() ).generic_string();

    adfs::folder dbf = fs.addFolder( pathname.wstring() );
    detail::import::attributes( dbf, folder.attributes() );

    std::wstring name = std::filesystem::path( source.filename() ).filename().wstring() + L":" + folium.name();
    portfolio::Folium xfolium( folium );
    xfolium.name( name );

    return detail::folium::save( dbf, pathname, source, xfolium );
}

// static
bool
fsio2::open( adfs::filesystem& fs, const std::wstring& filename )
{
    if ( !std::filesystem::exists( filename ) ) {
        if ( !fs.create( filename.c_str() ) )
            return false;
    } else {
        if ( ! fs.mount( filename.c_str() ) )
            return false;
    }
    return true;
}

bool
fsio2::append( adfs::filesystem& fs
               , const portfolio::Folium& folium
               , const adcontrols::datafile& source )
{
	portfolio::Folder folder = folium.parentFolder();
    std::filesystem::path pathname = ( std::filesystem::path( "/Processed" ) / folder.name() ).generic_string();

    adfs::folder dbf = fs.addFolder( pathname.wstring() );
    detail::import::attributes( dbf, folder.attributes() );

    auto uppath = std::filesystem::path( source.filename() ).parent_path().filename();
    auto stem = std::filesystem::path( source.filename() ).stem().string();
    std::smatch match;
    if ( std::regex_match( stem, match, std::regex( R"_(.*[^0-9]([0-9]+)$)_" ) ) ) {
        if ( match.size() == 2 )
            stem = match[1].str();
    }
    auto fname = ( boost::format("%1%_%2%__%3%") % uppath.string() % stem %  folium.name<char>() ).str();

    portfolio::Folium xfolium( folium );
    xfolium.name( fname );

    return detail::folium::save( dbf, pathname, source, xfolium );
}
