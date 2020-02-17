/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "export.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adportable/xml_serializer.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace adexport;

Export::~Export()
{
}

Export::Export()
{
}

bool
Export::open( const boost::filesystem::path& path )
{
    ADDEBUG() << path;

    auto fs = std::make_unique< adfs::filesystem >();

    if ( boost::filesystem::exists( path ) ) {
        if ( fs->mount( path ) ) {
            fs_ = std::move( fs );
            return true;
        }
    }
    return false;
}

bool
Export::loadFolders()
{
    for ( auto folder: fs_->root().folders() ) {
        boost::filesystem::path path( L"/" + folder.name() );
        list( path, folder, folders_ );
    }
    return true;
}

bool
Export::list( const boost::filesystem::path& path, const adfs::folder& folder, std::vector< adfs::folder >& flist ) const
{
    auto folders = folder.folders();

    for ( auto folder: folders ) {
        if ( folder.folders().empty() ) {
            flist.emplace_back( folder );
        } else {
            list( path / folder.name(), folder, flist );
        }
    }
    return !folders.empty();
}

bool
Export::out( const adfs::folder& folder, std::ostream& o ) const
{
    o << "## FILE: " << fs_->filename() << std::endl;
    o << "## FOLDER: " << folder.name<char>() << std::endl;
    for ( auto file: folder.files() ) {
        out( file, o, "## " );

        for ( auto afile: file.attachments() ) {
            out( file, o, "#\tATTACH\t" );
        }
        
    }    
    return true;
}

bool
Export::out( const adfs::file& file, std::ostream& o, const std::string& header ) const
{
    o << header << "NAME:\t"  << file.attribute( "name" ) << "\"" << std::endl;
    o << header << "ID:\t"    << adportable::utf::to_utf8( file.id() ) << "\"" << std::endl;
    o << header << "CLASS:\t" << adportable::utf::to_utf8( file.dataClass() ) << "\"" << std::endl;
    o << header << "rowid:\t" << file.rowid() << "\"" << std::endl;

    // src/adplugins/addatafile/datafile.cpp fetch method

    adfs::stmt sql( fs_->db() );
    sql.prepare( "SELECT data FROM file WHERE fileid=?" );
    sql.bind( 1 ) = file.rowid();

    if ( sql.step() == adfs::sqlite_row ) {
        auto blob = sql.get_column_value< adfs::blob >( 0 );
        if ( blob.size() ) {
            if ( file.dataClass() == adcontrols::MassSpectrum::dataClass() ) {
                auto ptr = std::make_shared< adcontrols::MassSpectrum >();
                try {
                    if ( adfs::cpio::deserialize( *ptr, reinterpret_cast< const char *>(blob.data()), blob.size() ) ) {
                        o << header << "size:\t" << ptr->size() << std::endl;
                        const auto& prop = ptr->getMSProperty();
                        const auto& info = prop.samplingInfo();
                        o << header << "sampInterval\t" << info.fSampInterval() << std::endl;
                        o << header << "nSamples\t" << info.nSamples() << std::endl;
                        o << header << "numberOfTriggers\t" << info.numberOfTriggers() << std::endl;
                        o << header << "mode\t" << info.mode() << std::endl;
                        o << header << "massSpectrometerClsid\t" << prop.massSpectrometerClsid() << std::endl;
                        
                        for ( size_t i = 0; i < ptr->size(); ++i ) {
                            o << i << "\t"
                              << std::fixed << std::setprecision( 14 )
                              << ptr->time(i) << "\t" << ptr->mass(i) << "\t" << ptr->intensity(i) << std::endl;
                        }
                    }
                } catch ( std::exception& e ) {
                    ADDEBUG() << "exception: " << e.what() << " while deserializing " << "MassSpectrum";
                }
            }
        }
    }
    return true;
}
