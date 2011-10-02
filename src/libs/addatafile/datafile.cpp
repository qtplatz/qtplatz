// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include "copyin_visitor.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/datapublisher.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/processeddataset.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <boost/any.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <adportable/string.hpp>
#include <adportable/posix_path.hpp>
#include <acewrapper/input_buffer.hpp>
#include <adfs/adfs.hpp>
#include <adfs/sqlite.hpp>
#include <algorithm>
#include <iostream>

/////////////////
namespace addatafile { namespace detail {

    static adcontrols::datafile * nullfile(0);

    struct attachment {
        static bool save( adfs::folium& parent, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
    };

    struct folder {
        static bool save( adfs::portfolio& db, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folder& );
    };

    struct folium {
        static bool save( adfs::folder&, const boost::filesystem::path&, const adcontrols::datafile&, const portfolio::Folium& );
    };


    struct import {

        static void attributes( portfolio::Folium dst, const adfs::folium& src ) {
            for ( adfs::internal::attributes::vector_type::const_iterator it = src.begin(); it != src.end(); ++it )
                dst.setAttribute( it->first, it->second );
            dst.setAttribute( L"rowid", boost::lexical_cast<std::wstring>( src.rowid() ) );
        }

        static void folium( portfolio::Folium dst, const adfs::folium& src ) {
            import::attributes( dst, src );
            adfs::folio attachments = src.attachments();
            for ( adfs::folio::const_iterator it = attachments.begin(); it != attachments.end(); ++it ) {
                portfolio::Folium att = dst.addAttachment( it->name() );
                import::folium( att, *it );
            }
        }

        static void folder( portfolio::Folder parent, const adfs::folder& adfolder ) {
            const adfs::folio adfolio = adfolder.folio();
            for ( adfs::folio::const_iterator it = adfolio.begin(); it != adfolio.end(); ++it ) {
                import::folium( parent.addFolium( it->name() ), *it );
            }
        }
    };

}
}

using namespace addatafile;

datafile::~datafile()
{
}

datafile::datafile() : mounted_(false)
{
}

void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    if ( mounted_ ) {
        // subscribe acquired dataset <LCMSDataset>
        do {
            sub.subscribe( *this );
        } while (0);

        // subscribe processed dataset
        do {
            portfolio::Portfolio portfolio;
            portfolio.create_with_fullpath( filename_ );

            processedDataset_->xml( portfolio.xml() );
            adfs::folder processed = dbf_.findFolder( L"/Processed/Spectra" );
            if ( processed )
                detail::import::folder( portfolio.addFolder( L"Spectra" ), processed );

            if ( processedDataset_ )
                sub.subscribe( *processedDataset_ );
        } while (0);
    }
}

bool
datafile::open( const std::wstring& filename, bool /* readonly */ )
{
    filename_ = filename;
    processedDataset_.reset( new adcontrols::ProcessedDataset );

    if ( ( mounted_ = dbf_.mount( filename.c_str() ) ) ) 
        return loadContents();

    if ( ( mounted_ = dbf_.create( filename.c_str() ) ) )
        return true;

    return false;
}

bool
datafile::open_qtms( const std::wstring& filename, bool /* readonly */ )
{
    // create a file mapping
    boost::interprocess::file_mapping map( adportable::string::convert(filename).c_str(), boost::interprocess::read_only );

    // map the whole file with read-only permissions in this process
    boost::interprocess::mapped_region region( map, boost::interprocess::read_only );
    std::size_t size = region.get_size();

    acewrapper::input_buffer ibuf( static_cast<unsigned char *>(region.get_address()), size );
    std::istream in( &ibuf );

    adcontrols::MassSpectrumPtr pMS( new adcontrols::MassSpectrum );
    adcontrols::MassSpectrum::restore( in, *pMS );
    data_ = pMS;     
    //-------------

    portfolio::Portfolio portfolio;

    portfolio.create_with_fullpath( filename );
    portfolio::Folder spectra = portfolio.addFolder( L"Spectra" );

    //----
    portfolio::Folium folium = spectra.addFolium( filename );
    folium.setAttribute( L"dataType", L"MassSpectrum" );
    folium.setAttribute( L"path", L"/" );
    //----
    processedDataset_.reset( new adcontrols::ProcessedDataset );
    processedDataset_->xml( portfolio.xml() );

    return true;
}

boost::any
datafile::fetch( const std::wstring& path, const std::wstring& dataType ) const
{
    (void)path;
    (void)dataType;
    return data_;
}

size_t
datafile::getSpectrumCount( int /* fcn */ ) const
{
    return 1;
}

bool
datafile::getSpectrum( int /* fcn */, int /* idx */, adcontrols::MassSpectrum& ) const
{
    return true;
}

bool
datafile::getTIC( int /* fcn */, adcontrols::Chromatogram& ) const
{
    return false;
}

size_t
datafile::getChromatogramCount() const
{
    return 0;
}

size_t
datafile::getFunctionCount() const
{
    return 1;
}


////////////////////////////////////////////////////

// SaveFileAs come in here
bool
datafile::saveContents( const std::wstring& path, const portfolio::Portfolio& portfolio, const adcontrols::datafile& source )
{
    if ( ! mounted_ )
        return false;

    adfs::stmt sql( dbf_.db() );
    sql.begin();

    dbf_.addFolder( path );

    adportable::path name( path );

    BOOST_FOREACH( const portfolio::Folder& folder, portfolio.folders() )
        detail::folder::save( dbf_, name, source, folder );
    //const std::vector< portfolio::Folder > folders = portfolio.folders();
    //std::for_each( folders.begin(), folders.end(), detail::saveFolder( dbf_, name, source ) );
    sql.commit();
    return true;
}

bool
datafile::saveContents( const std::wstring& path, const portfolio::Portfolio& portfolio )
{
    if ( ! mounted_ )
        return false;

    adfs::stmt sql( dbf_.db() );
    sql.begin();

    dbf_.addFolder( path );

    adportable::path name( path );

    BOOST_FOREACH( const portfolio::Folder& folder, portfolio.folders() )
        detail::folder::save( dbf_, name, *detail::nullfile, folder );

    sql.commit();
    return true;
}

bool
datafile::loadContents()
{
    if ( ! mounted_ )
        return false;

    std::vector< adfs::folder > folders = dbf_.folders();
        
    
    

    return true;
}

///-------------------------------------------------------------------------------------

namespace addatafile {
    namespace detail {

        bool
        attachment::save( adfs::folium& parent, const boost::filesystem::path& path, const adcontrols::datafile& source, const portfolio::Folium& folium )
        {
            boost::filesystem::path filename = adportable::path::posix( path / folium.id() );

            adfs::folium dbThis = parent.addAttachment( folium.id() );
            boost::any any = static_cast<const boost::any&>( folium );
            if ( any.empty() && (&source != nullfile ) )
                any = source.fetch( folium.path(), folium.dataClass() );
            detail::copyin_visitor::apply( any, dbThis );
            
            BOOST_FOREACH( const portfolio::Folium& att, folium.attachments() )
                save( dbThis, filename, source, att );
            return true;
        }
        //------------

        bool
        folium::save( adfs::folder& folder, const boost::filesystem::path& path, const adcontrols::datafile& source, const portfolio::Folium& folium )
        {
            boost::filesystem::path filename = adportable::path::posix( path / folium.id() );

            // get attributes
            std::vector< std::pair< std::wstring, std::wstring > > attrs = folium.attributes();

            boost::any any = static_cast<const boost::any&>( folium );
            if ( any.empty() && (&source != nullfile ) )
                any = source.fetch( folium.path(), folium.dataClass() );

            if ( folder ) {
                adfs::folium dbf = folder.addFolium( folium.id() );
                detail::copyin_visitor::apply( any, dbf );

                BOOST_FOREACH( const portfolio::Folium& att, folium.attachments() )
                    detail::attachment::save( dbf, filename, source, att );
            }
            return true;
        }

        // struct folder {
        bool
        folder::save( adfs::portfolio& dbf, const boost::filesystem::path& path, const adcontrols::datafile& source, const portfolio::Folder& folder )
        {
            boost::filesystem::path pathname = adportable::path::posix( path / folder.name() );

            adfs::folder dbThis = dbf.addFolder( pathname.wstring() );

            // save all files in this folder
            BOOST_FOREACH( const portfolio::Folium& folium, folder.folio() )
                folium::save( dbThis, pathname, source, folium );
    
            // recursive save sub folders
            BOOST_FOREACH( const portfolio::Folder& subfolder, folder.folders() )
                folder::save( dbf, pathname, source, subfolder );

            return true;
        }
        //---------

    }
}

//////////////////////////
