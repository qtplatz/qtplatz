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

#include "attributes.h"
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#pragma warning (disable:4996)
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#pragma warning (default:4996)

#include <boost/smart_ptr.hpp>
#include "adsqlite.h"
#include "streambuf.h"

using namespace adfs;
using namespace adfs::internal;

attributes::attributes() : dirty_( false )
{
}

attributes::attributes( const attributes& t ) : attrib_( t.attrib_ )
                                              , dirty_( t.dirty_ ) 
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
    boost::archive::binary_oarchive ar( os );
    ar << impl;
    return true;
}

bool
attributes::restore( attributes& impl, std::istream& is ) // binary
{
    boost::archive::binary_iarchive ar( is );
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
                adfs::istreambuf ibuf( p.get(), blob.size() );
                std::istream in( &ibuf );
                if ( restore( *this, in ) )
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
        adfs::ostreambuf obuf;
        std::ostream out( &obuf );
        if ( archive( out, *this ) ) {
            adfs::stmt sql( db() );
            if ( sql.prepare( "UPDATE directory SET attr = :attr WHERE rowid = :rowid" ) ) {
                sql.bind( 1 ) = blob( obuf.size(), obuf.p() );
                sql.bind( 2 ) = rowid();
                if ( sql.step() == adfs::sqlite_done )
                    dirty_ = false;
            }
        }
    }
    return ! dirty_;
}