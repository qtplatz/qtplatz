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
#include <adportable/debug.hpp>
#include <acewrapper/input_buffer.hpp>
#include <adfs/adfs.hpp>
#include <adfs/attributes.hpp>
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
        static void attributes( adfs::internal::attributes&, const portfolio::attributes_type& );
        static void attributes( portfolio::Folium&, const adfs::internal::attributes& );
        static void attributes( portfolio::Folder&, const adfs::internal::attributes& );
        static void folium( portfolio::Folium dst, const adfs::folium& src );
        static void folder( portfolio::Folder parent, const adfs::folder& adfolder );
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
            if ( loadContents( portfolio, L"/Processed" ) && processedDataset_ ) {
                processedDataset_->xml( portfolio.xml() );
                sub.subscribe( *processedDataset_ );
            }
        } while (0);
    }
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
    adportable::debug() << "datafile::fetch(" << path << ", " << dataType << ")";
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

    sql.commit();
    return true;
}


// Save comes here. Thus no file name pass to here
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
datafile::loadContents( portfolio::Portfolio& portfolio, const std::wstring& query )
{
    if ( ! mounted_ )
        return false;

    portfolio.create_with_fullpath( filename_ );
    adfs::folder processed = dbf_.findFolder( query );  // L"/Processed"
    if ( ! processed )
        return false;

    // top folder should be L"Spectra" | L"Chromatogram"
    BOOST_FOREACH( const adfs::folder& folder, processed.folders() ) {
        const std::wstring& name = folder.name();
        detail::import::folder( portfolio.addFolder( name ), folder );
    }

    processedDataset_.reset( new adcontrols::ProcessedDataset );
    processedDataset_->xml( portfolio.xml() );

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
            import::attributes( dbThis, folium.attributes() );

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

            boost::any any = static_cast<const boost::any&>( folium );
            if ( any.empty() && (&source != nullfile ) )
                any = source.fetch( folium.path(), folium.dataClass() );

            if ( folder ) {
                adfs::folium dbf = folder.addFolium( folium.id() );

                import::attributes( dbf, folium.attributes() );
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
            import::attributes( dbThis, folder.attributes() );

            // save all files in this folder
            BOOST_FOREACH( const portfolio::Folium& folium, folder.folio() )
                folium::save( dbThis, pathname, source, folium );
    
            // recursive save sub folders
            BOOST_FOREACH( const portfolio::Folder& subfolder, folder.folders() )
                folder::save( dbf, pathname, source, subfolder );

            return true;
        }

        void import::folder( portfolio::Folder parent, const adfs::folder& adfolder )
        {
            BOOST_FOREACH( const adfs::folium& folium, adfolder.folio() )
                import::folium( parent.addFolium( folium.name() ), folium );
        }

        void import::folium( portfolio::Folium dst, const adfs::folium& src )
        {
            import::attributes( dst, src );
            BOOST_FOREACH( const adfs::folium& attachment, src.attachments() )
                import::attributes( dst.addAttachment( attachment.name() ), attachment );
        }

        //---
        void import::attributes( portfolio::Folium& d, const adfs::internal::attributes& s )
        {
            for ( adfs::internal::attributes::vector_type::const_iterator it = s.begin(); it != s.end(); ++it )
                d.setAttribute( it->first, it->second );
            d.setAttribute( L"rowid", boost::lexical_cast<std::wstring>( s.rowid() ) );
        }

        void import::attributes( portfolio::Folder& d, const adfs::internal::attributes& s )
        {
            for ( adfs::internal::attributes::vector_type::const_iterator it = s.begin(); it != s.end(); ++it )
                d.setAttribute( it->first, it->second );
            d.setAttribute( L"rowid", boost::lexical_cast<std::wstring>( s.rowid() ) );
        }

        void import::attributes( adfs::internal::attributes& d, const portfolio::attributes_type& s )
        {
            BOOST_FOREACH( const portfolio::attribute_type& a, s ) 
                d.setAttribute( a.first, a.second );
        }
        //---

    }
}

//////////////////////////
