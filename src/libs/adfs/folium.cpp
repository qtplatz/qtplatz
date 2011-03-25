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

#include "folium.h"
#include "folder.h"
#include "portfolioimpl.h"
#include "filesystem.h"
#include "streambuf.h"

using namespace adfs;

folium::~folium()
{
}

folium::folium() : db_(0)
                 , rowid_(0)
                 , is_attachment_(false)
{
}


folium::folium( const folium& t ) : db_( t.db_ )
                                  , rowid_( t.rowid_ )
                                  , name_( t.name_ )  
                                  , is_attachment_( t.is_attachment_ ) 
{
}

folium::folium( sqlite& db, boost::int64_t rowid
               , const std::wstring& name
               , bool is_attachment ) : db_( &db )
                                      , rowid_( rowid )
                                      , name_( name )  
                                      , is_attachment_( is_attachment ) 
{
}

std::wstring
folium::path() const
{
    // path is the unique id to open a data
    return name_; // attribute( L"path" );
}

bool
folium::empty() const
{
/*
    if ( impl_ ) {
        boost::any& data = impl_->find( id() );
        return data.empty();
    }
*/
    return true;
}

void
folium::operator = ( boost::any& any )
{
/*
    if ( impl_ )
        impl_->assign( id(), any );
*/
}

folium::operator boost::any & ()
{
/*
    if ( impl_ )
        return impl_->find( id() );
*/
    static boost::any temp;
    return temp;
}

folio
folium::attachments()
{
    folio attachments;

    adfs::stmt sql( *db_ );
    sql.prepare( "select rowid, name from directory where type = 3 and parent_id = ?" );
    sql.bind( 1 ) = rowid_;

    while ( sql.step() == adfs::sqlite_row ) {
        attachments.push_back( adfs::folium(
                                              *db_
                                              , boost::get< boost::int64_t >( sql.column_value( 0 ) )
                                              , boost::get< std::wstring >( sql.column_value( 1 ) ) ) );
    }
    return attachments;
}

folium
folium::addAttachment( const std::wstring& name )
{
    return internal::fs::add_attachment( *this, name );
}

folder
folium::getParentFolder()
{
    return internal::fs::get_parent_folder( *db_, rowid_ );
}

std::size_t
folium::write( std::size_t size, const unsigned char * p, std::size_t offs )
{
    (void)offs;
    if ( internal::fs::write( *db_, rowid_, size, p ) )
        return size;

    return 0;
}

std::size_t
folium::write( const adfs::streambuf& buffer, std::size_t offs )
{
    (void)offs;
    if ( internal::fs::write( *db_, rowid_, buffer.size(), buffer.p() ) )
        return buffer.size();
    return 0;
}
