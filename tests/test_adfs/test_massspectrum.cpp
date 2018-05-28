/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
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

#include "test_massspectrum.hpp"

#include <adcontrols/massspectrum.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/file.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/xml_serializer.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>

namespace test_adfs {

    constexpr static const char * const adfs_file = "massspectrum1.adfs";
    
    void
    make_massspectrum( adcontrols::MassSpectrum& m )
    {
        m.resize( 1000 );
    }
        
    bool
    write_massspectrum( const adcontrols::MassSpectrum& m, const std::string& name )
    {
        std::ofstream fo( name );
        return adportable::binary::serialize<>()( m, fo );
    }
        
    bool
    read_massspectrum( adcontrols::MassSpectrum& m, const std::string& name )
    {
        std::ifstream fi( name );
        if ( !fi.fail() )
            return adportable::binary::deserialize<>()( m, fi );
        return false;
    }
}

using namespace test_adfs;

bool
massspectrum::test()
{
    BOOST_TEST_CHECKPOINT("massspectrum::test!");

    ::adfs::filesystem fs;

    if ( boost::filesystem::exists( adfs_file ) ) {
        if ( ! fs.mount( adfs_file ) )
            return false;
    } else {
        if ( ! fs.create( adfs_file ) )
            return false;
    }

	auto folder = fs.addFolder( L"/Processed/Spectra" );

    adcontrols::MassSpectrum ms;
    make_massspectrum( ms );

    if ( folder ) {
		adfs::file file = folder.addFile( adfs::create_uuid(), L"title" );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            auto id = file.id();
            if ( file.save( ms ) ) //adfs::cpio< adcontrols::MassSpectrum >::save( ms, file ) )
				file.commit();
        }
	}
    return true;
    // return adportable::xml::serialize<>()( t, xml );
}

bool
massspectrum::test_create()
{
    BOOST_TEST_CHECKPOINT("massspectrum::test_create!");

    ::adfs::filesystem fs;

    if ( boost::filesystem::exists( adfs_file ) ) 
        boost::filesystem::remove( adfs_file );
    if ( ! fs.create( adfs_file ) )
        return false;

	auto folder = fs.addFolder( L"/Processed/Spectra" );

    adcontrols::MassSpectrum ms;
    make_massspectrum( ms );

    if ( folder ) {
		adfs::file file = folder.addFile( adfs::create_uuid(), L"title" );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            auto id = file.id();
            if ( file.save( ms ) ) //adfs::cpio< adcontrols::MassSpectrum >::save( ms, file ) )
				file.commit();
        }
	}
    return true;
    // return adportable::xml::serialize<>()( t, xml );
}

bool
massspectrum::test_read()
{
    BOOST_TEST_CHECKPOINT("massspectrum::test_read!");
    
    ::adfs::filesystem fs;

    if ( !boost::filesystem::exists( adfs_file ) )
        return false;

    if ( ! fs.mount( adfs_file ) )
        return false;

    BOOST_TEST_MESSAGE( "read test: " << adfs_file );
    
    if ( auto folder = fs.findFolder( L"/Processed/Spectra" ) ) {
        BOOST_TEST_MESSAGE( "folders size: " << folder.folders().size() );
        for ( auto subfolder: folder.folders() ) {
            ADDEBUG() << folder.name();
            BOOST_TEST_MESSAGE( "folder.name: " << adportable::utf::to_utf8( folder.name() ) );
        }
        return true;
    }

    return false;
}

bool
massspectrum::test_read( const char * adfs_file )
{
    BOOST_TEST_CHECKPOINT("massspectrum::test_read_2!");
    const boost::filesystem::path path( adfs_file );
    const boost::filesystem::path xmlpath = boost::filesystem::path( adfs_file ).replace_extension( "xml" );
    
    ::adfs::filesystem fs;

    if ( !boost::filesystem::exists( path ) )
        return false;

    if ( ! fs.mount( path ) )
        return false;

    BOOST_TEST_MESSAGE( "read test: " << path.string() );
    
    if ( auto folder = fs.findFolder( L"/Processed/Spectra" ) ) {
        BOOST_TEST_MESSAGE( "folder.name: " << adportable::utf::to_utf8( folder.name() ) );
        BOOST_TEST_MESSAGE( "folders size: " << folder.folders().size() );
        BOOST_TEST_MESSAGE( "files size: " << folder.files().size() );
        for ( auto file: folder.files() ) {
            BOOST_TEST_MESSAGE( "found file identified: " << file.name< char >() << " class=" << adportable::utf::to_utf8( file.dataClass() ) );
            if ( file.dataClass() == adcontrols::MassSpectrum::dataClass() ) {
                adcontrols::MassSpectrum ms;
                BOOST_CHECK( file.fetch( ms ) );
                std::wofstream xml( xmlpath.string().c_str() );
                BOOST_CHECK( adportable::xml::serialize<>()( ms, xml ) );
            }
            for ( auto a: file.attachments() ) {
                BOOST_TEST_MESSAGE( "found attachment identified: " << a.name< char >() );
            }
        }
        return true;
    }

    return false;
}
