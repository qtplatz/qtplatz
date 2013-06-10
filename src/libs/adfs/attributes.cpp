// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include <boost/smart_ptr.hpp>
#include "sqlite.hpp"

using namespace adfs;
using namespace adfs::internal;

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
    portable_binary_iarchive ar( is );
    ar >> impl;
    return true;
}


/*
bool
attributes::archive( std::wostream& os ) const  // xml
{
    boost::archive::xml_woarchive ar ( os );
    ar << *this;
    return true;
}

bool
attributes::restore( std::wistream& is ) // xml
{
    boost::archive::xml_wiarchive ar( is );
    ar >> is;
    return true;
}
*/

bool
attributes::fetch()
{
    adfs::blob blob;
    
    if ( rowid() && blob.open( db(), "main", "directory", "attr", rowid(), adfs::readonly ) ) {
        if ( blob.size() ) {
            boost::scoped_array< boost::int8_t > p( new boost::int8_t [ blob.size() ] );
            if ( blob.read( p.get(), blob.size() ) ) {
                adfs::detail::cpio obuf( blob.size(), reinterpret_cast<adfs::char_t *>( p.get() ) );
                if ( adfs::cpio<attributes>::copyout( *this, obuf ) )
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
        adfs::detail::cpio ibuf;
        if ( adfs::cpio<attributes>::copyin( *this, ibuf ) ) {
            adfs::stmt sql( db() );
            if ( sql.prepare( "UPDATE directory SET attr = :attr WHERE rowid = :rowid" ) ) {
                sql.bind( 1 ) = blob( ibuf.size(), reinterpret_cast<const boost::int8_t *>( ibuf.get() ) );
                sql.bind( 2 ) = rowid();
                if ( sql.step() == adfs::sqlite_done )
                    dirty_ = false;
            }
        }
    }
    return ! dirty_;
}
