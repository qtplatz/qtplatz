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

#include "datafile.h"
#include <xmlwrapper/msxml.h>
#include <adcontrols/datafile.h>
#include <adcontrols/datapublisher.h>
#include <adcontrols/datasubscriber.h>
#include <adcontrols/massspectrum.h>
#include <adcontrols/processeddataset.h>
#include <portfolio/portfolio.h>
#include <portfolio/folder.h>
#include <portfolio/folium.h>
#include <boost/any.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <adportable/string.h>
#include <acewrapper/input_buffer.h>
#include <adfs/adfs.h>
#include <algorithm>
#include <iostream>

using namespace addatafile;

datafile::~datafile()
{
}

datafile::datafile()
{
}

void
datafile::accept( adcontrols::dataSubscriber& sub ) const
{
    // subscribe acquired dataset <LCMSDataset>
    sub.subscribe( *this );

    // subscribe processed dataset
    if ( processedDataset_ )
        sub.subscribe( *processedDataset_ );
}

bool
datafile::open( const std::wstring& filename, bool /* readonly */ )
{
    portfolio::Portfolio portfolio;
    portfolio.create_with_fullpath( filename );

    struct import {

        static void attributes( portfolio::Folium& dst, const adfs::folium& src ) {
            for ( adfs::internal::attributes::vector_type::const_iterator it = src.begin(); it != src.end(); ++it )
                dst.setAttribute( it->first, it->second );
            dst.setAttribute( L"rowid", boost::lexical_cast<std::wstring>( src.rowid() ) );
        }

        static void folium( portfolio::Folium& dst, const adfs::folium& src ) {
            import::attributes( dst, src );
            adfs::folio attachments = src.attachments();
            for ( adfs::folio::const_iterator it = attachments.begin(); it != attachments.end(); ++it ) {
                portfolio::Folium att = dst.addAttachment( it->name() );
                import::folium( att, *it );
            }
        }

        static void folder( portfolio::Folder& parent, const adfs::folder& adfolder ) {
            const adfs::folio adfolio = adfolder.folio();
            for ( adfs::folio::const_iterator it = adfolio.begin(); it != adfolio.end(); ++it ) {
                import::folium( parent.addFolium( it->name() ), *it );
            }
        }
    };

    adfs::portfolio file;
    if ( file.mount( filename.c_str() ) ) {
        adfs::folder processed = file.findFolder( L"/Processed/Spectra" );
        if ( processed ) {
            import::folder( portfolio.addFolder( L"Spectra" ), processed );
        }
    }

    processedDataset_.reset( new adcontrols::ProcessedDataset );
    processedDataset_->xml( portfolio.xml() );

    return true;
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
    pMS->restore( in );
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

/////////////////
namespace addatafile { namespace detail {

    class datafile {
    public:
        static bool saveFolder( const boost::filesystem::path&, const portfolio::Folder&, const adcontrols::datafile& );
    };

    struct saveFolium {
        bool operator () ( const portfolio::Folder& folder ) {
            return true;
        }
    };

    struct saveFolder {
        const boost::filesystem::path& path;
        const adcontrols::datafile& source;
        saveFolder( const boost::filesystem::path& p, const adcontrols::datafile& f ) : path(p), source(f) {}
        bool operator () ( const portfolio::Folder& folder );
    };

}
}

bool
datafile::saveContents( const std::wstring& path, const portfolio::Portfolio& portfolio, const adcontrols::datafile& source )
{
    boost::filesystem::path name( path );
    const std::vector< portfolio::Folder > folders = portfolio.folders();

    std::for_each( folders.begin(), folders.end(), detail::saveFolder( name, source ) );
/*
    for ( std::vector< portfolio::Folder >::const_iterator it = folders.begin(); it != folders.end(); ++it )
        detail::datafile::saveFolder( name / it->name(), *it, source );
*/
    return true;
}

bool
detail::saveFolder::operator () ( const portfolio::Folder& folder )
{
    boost::filesystem::wpath p = path / folder.name();
    std::wstring nname = p.normalize().wstring();
    std::wcout << L"saveFolder: " << ( path / folder.name() ).wstring() << std::endl;

    const std::vector< portfolio::Folder > folders = folder.folders();
    std::for_each( folders.begin(), folders.end(), detail::saveFolder( path / folder.name(), source ) );

    return true;
}

