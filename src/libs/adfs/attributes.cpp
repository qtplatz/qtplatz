// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "attributes.hpp"
#include "cpio.hpp"

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <compiler/diagnostic_pop.h>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/debug.hpp>
#include <boost/exception/all.hpp>
#include "sqlite.hpp"

using namespace adfs;

attributes::attributes() : dirty_( false )
{
}

attributes::attributes( const attributes& t ) : dirty_( t.dirty_ ) 
                                              , attrib_( t.attrib_ )
{
}

attributes::operator bool () const 
{
    return &db() != 0 && rowid() != 0;
}

std::wstring
attributes::name() const
{
    return attribute( L"name" );
}

void
attributes::name( const std::wstring& value )
{
    setAttribute( L"name", value );
}

std::wstring
attributes::id() const
{
    return attribute( L"dataId" );
}

void
attributes::id( const std::wstring& value )
{
   setAttribute( L"dataId", value );
}

std::wstring
attributes::dataClass() const
{
    return attribute( L"dataType" );
}

void
attributes::dataClass( const std::wstring& value )
{
    setAttribute( L"dataType", value );
}

std::wstring
attributes::attribute( const std::wstring& key ) const
{
    std::map<std::wstring, std::wstring>::const_iterator it = attrib_.find( key );
    if ( it != attrib_.end() )
        return it->second;
    return std::wstring();
}

void
attributes::setAttribute( const std::wstring& key, const std::wstring& value )
{
    dirty_ = true;
    attrib_[ key ] = value;
}

//////////////////////////

bool
attributes::archive( std::ostream& os, const attributes& impl )
{
    portable_binary_oarchive ar( os );
    ar << impl;
    return true;
}

bool
attributes::restore( std::istream& is, attributes& impl ) // binary
{
    try {
        portable_binary_iarchive ar( is );
        ar >> impl;
    } catch ( boost::archive::archive_exception& ex ) {
        ADDEBUG() << "archive_exception code=" << ex.code << "\t" << boost::diagnostic_information( ex );
        return false;
    } catch ( std::exception& ex ) {
        ADDEBUG() << boost::diagnostic_information( ex );
        return false;
    }
    return true;
}

bool
attributes::fetch()
{
    adfs::blob blob;
    
    if ( rowid() && blob.open( db(), "main", "directory", "attr", rowid(), adfs::readonly ) ) {
        if ( blob.size() ) {
			std::unique_ptr< boost::int8_t [] > p( new boost::int8_t [ blob.size() ] );
            if ( blob.read( p.get(), blob.size() ) ) {
                if ( adfs::cpio::deserialize( *this, reinterpret_cast<const char *>( p.get() ), blob.size() ) )
                    dirty_ = false;
            }
        }
    }
    return ! dirty_;
}

bool
attributes::commit()
{
    if ( dirty_ ) {
        std::string device;
        if ( adfs::cpio::serialize( *this, device ) ) {

            uint32_t format_version = fetch_format_version();

            adfs::stmt sql( db() );
            if ( format_version >= 4 ) {
                if ( sql.prepare( "UPDATE directory SET attr = ?, display_name = ?, dataclass = ? WHERE rowid = ?" ) ) {
                    sql.bind( 1 ) = blob( device.size(), reinterpret_cast<const int8_t *>( device.data() ) );
                    sql.bind( 2 ) = this->name();
                    sql.bind( 3 ) = this->dataClass();
                    sql.bind( 4 ) = rowid();
                    if ( sql.step() == adfs::sqlite_done )
                        dirty_ = false;
                }
            } else {
                if ( sql.prepare( "UPDATE directory SET attr = ? WHERE rowid = ?" ) ) {
                    sql.bind( 1 ) = blob( device.size(), reinterpret_cast<const int8_t *>( device.data() ) );
                    sql.bind( 2 ) = rowid();
                    if ( sql.step() == adfs::sqlite_done )
                        dirty_ = false;
                }
            }
        }
    }
    return ! dirty_;
}

uint32_t
attributes::fetch_format_version() const
{
    uint32_t format_version = 0;
    if ( ( format_version = db().fs_format_version() ) )
        return format_version;
    
    adfs::stmt sql( db() );
    if ( sql.prepare( "PRAGMA TABLE_INFO(directory)" ) ) {
        while ( sql.step() == adfs::sqlite_row ) {
            if ( sql.get_column_value< std::string >( 1 ) == "display_name" )
                return 4;
        }
        return 3;
    }
    return 0;
}
