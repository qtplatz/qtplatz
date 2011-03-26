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

#include "folder.h"
#include "folium.h"
#include "filesystem.h"

using namespace adfs;

folder::~folder()
{
}

folder::folder() : db_( 0 ), rowid_( 0 )
{
}

folder::folder( const folder& t ) : db_( t.db_ )
                                  , rowid_( t.rowid_ )
                                  , name_( t.name_ )
                                  , attributes( t ) 
{
}

folder::folder( adfs::sqlite& db, boost::int64_t rowid, const std::wstring& name ) : db_( &db )
                                                                                   , rowid_( rowid )
                                                                                   , name_( name )
{
}

std::vector< folder >
folder::folders()
{
    std::vector< folder > folders;
    if ( db_ && rowid_ )
        internal::fs::select_folders( *db_, rowid_, folders );
    return folders;
}

adfs::folio
folder::folio()
{
    adfs::folio folio;
    if ( db_ && rowid_ )
        internal::fs::select_folio( *db_, rowid_, folio );
    return folio;
}

folium
folder::selectFolium( const std::wstring& )
{
    return folium();
}

/////////////////////////
folium
folder::addFolium( const std::wstring& name )
{
    if ( db_ && rowid_ )
        return internal::fs::add_folium( *this, name );
    return folium();
}
