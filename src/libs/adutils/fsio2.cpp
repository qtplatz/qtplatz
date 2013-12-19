/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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
#include "adfsio.hpp"
#include "cpio.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adinterface/method.hpp>
#include <adfs/cpio.hpp>
#include <adportable/debug.hpp>

#include <adfs/sqlite.hpp>
#include <adportable/posix_path.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>

namespace adutils { namespace detail {

    static adcontrols::datafile * nullfile(0);

        struct folder {
            static bool save( adfs::filesystem& db, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folder& );
            static bool load( portfolio::Folder parent, const adfs::folder& adf );
        };

        struct folium {
            static bool save( adfs::folder&, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
            static bool load( portfolio::Folium dst, const adfs::file& src );
        };

        struct attachment {
            static bool save( adfs::file& parent, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
            static bool load( portfolio::Folium dst, const adfs::file& adf );
        };

        struct import {
            static void attributes( adfs::attributes&, const portfolio::attributes_type& );
            static void attributes( portfolio::Folium&, const adfs::attributes& );
            static void attributes( portfolio::Folder&, const adfs::attributes& );
        };

        template< class T > struct serializer {
            static std::shared_ptr< T > deserialize( adfs::detail::cpio& obuf ) {
                std::shared_ptr<T> ptr = std::make_shared<T>();
                try {
                    adfs::cpio<T>::deserialize( *ptr, obuf );
                } catch ( std::exception& e ) {
                    adportable::debug(__FILE__, __LINE__) << "exception: " << e.what() << " while deserializing " << typeid(T).name();
                    ptr.reset();
                }
                return ptr;
            }
        };

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

    adportable::path name( path );

    for ( const portfolio::Folder& folder: portfolio.folders() )
        detail::folder::save( dbf, name, source, folder );

    sql.commit();

    return true;
}


///////////////////////////////
bool
detail::attachment::save( adfs::file& parent, const boost::filesystem::path& path
                          , const adcontrols::datafile& source, const portfolio::Folium& folium )
{
    boost::filesystem::path filename = adportable::path::posix( path / folium.id() );
    
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
        } catch ( boost::bad_any_cast& ) {
            assert( 0 );
            return false;
        }
        
        for ( const portfolio::Folium& att: folium.attachments() )
            save( dbThis, filename, source, att );
    }
    return true;
}

bool
detail::folium::save( adfs::folder& folder, const boost::filesystem::path& path
                      , const adcontrols::datafile& source, const portfolio::Folium& folium )
{
    boost::filesystem::path filename = adportable::path::posix( path / folium.id() );

    boost::any any = static_cast<const boost::any&>( folium );
    if ( any.empty() && (&source != nullfile ) )
        any = source.fetch( folium.id(), folium.dataClass() );

    if ( folder && !any.empty() ) {
        adfs::file dbf = folder.addFile( folium.id() );

        import::attributes( dbf, folium.attributes() );
        cpio::save( dbf, any );

        for ( const portfolio::Folium& att: folium.attachments() )
            detail::attachment::save( dbf, filename, source, att );
    }
    return true;
}

bool
detail::folder::save( adfs::filesystem& dbf, const boost::filesystem::path& path
                      , const adcontrols::datafile& source, const portfolio::Folder& folder )
{
    boost::filesystem::path pathname = adportable::path::posix( path / folder.name() );

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

void
detail::import::attributes( adfs::attributes& d, const portfolio::attributes_type& s )
{
    for ( const portfolio::attribute_type& a: s ) 
        d.setAttribute( a.first, a.second );
}

// static
bool
fsio2::appendOnFile( const std::wstring& filename, const portfolio::Folium& folium, const adcontrols::datafile& source )
{
    boost::filesystem::path path( filename );

    adfs::filesystem fs;
    
    if ( !boost::filesystem::exists( path ) ) {
        if ( !fs.create( path.wstring().c_str() ) )
            return false;
    } else {
        if ( ! fs.mount( path.wstring().c_str() ) )
            return false;
    }

    ADDEBUG() << folium.name();
    
/*
    adfs::stmt sql( dbf.db() );
    sql.begin();

    dbf.addFolder( path );

    adportable::path name( path );

    for ( const portfolio::Folder& folder: portfolio.folders() )
        detail::folder::save( dbf, name, source, folder );

    sql.commit();
*/
    

    // if ( adfs::folder folder = fs.addFolder( L"/Processed/Spectra" ) ) {
        

    // }

    return true;
}

