// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "folium.hpp"
#include "folder.hpp"
#include "portfolioimpl.hpp"
#include "filesystem.hpp"
#include <adportable/debug.hpp>
#include "attributes.hpp"

using namespace adfs;

folium::~folium()
{
}

folium::folium() : db_(0)
                 , rowid_(0)
                 , is_attachment_(false)
{
}


folium::folium( const folium& t ) : attributes( t )
                                  , db_( t.db_ )
                                  , name_( t.name_ )  
                                  , rowid_( t.rowid_ )
                                  , is_attachment_( t.is_attachment_ )
{
}

folium::folium( sqlite& db
               , boost::int64_t rowid
               , const std::wstring& name
                , bool is_attachment ) : db_( &db )
                                       , name_( name )  
                                       , rowid_( rowid )
                                       , is_attachment_( is_attachment ) 
{
    fetch();
}


folio
folium::attachments()
{
    folio attachments;

    adfs::stmt sql( *db_ );
    sql.prepare( "SELECT rowid, name FROM directory WHERE type = 3 AND parent_id = :parent_id" );
    sql.bind( 1 ) = rowid_;

    while ( sql.step() == adfs::sqlite_row ) {
        boost::int64_t rowid = boost::get< boost::int64_t >( sql.column_value( 0 ) );
        std::wstring name = boost::get< std::wstring>( sql.column_value( 1 ) );
        attachments.push_back( adfs::folium( *db_, rowid, name ) );
#if defined DEBUG
        const folium& v = attachments.back();
        std::wcerr << L" --> select attachments: " << name << std::endl << L"\t\t";
        std::wcerr << L"(" << v.attribute(L"dataType") << L", " << v.attribute( L"name" ) << ")";
        std::wcerr << std::endl;
#endif
    }
    return attachments;
}

const folio
folium::attachments() const
{
    return const_cast<adfs::folium&>(*this).attachments();
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
folium::write( std::size_t size, const char_t * p )
{
    if ( internal::fs::write( *db_, rowid_, size, p ) ) {
        commit(); // commit attribute -- TODO: this api is confusing, consider change!!! TH 14th Nov 2011
        return size;
    }
    return 0;
}

std::size_t
folium::read( std::size_t size, char_t * p )
{
    if ( internal::fs::read( *db_, internal::fs::rowid_from_fileid( *db_, rowid_ ), size, p ) )
        return size;
    return 0;
}

std::size_t
folium::size() const
{
    size_t size = internal::fs::size( *db_, internal::fs::rowid_from_fileid( *db_, rowid_ ) );
    return size;
}
