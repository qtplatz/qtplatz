/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include <xmlparser/pugixml.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

using namespace adpublisher;

document::document() : doc_( std::make_shared< pugi::xml_document >() )
{
    if ( auto comment = doc_->append_child( pugi::node_comment ) ) {
        comment.set_value( "Copyright(C) 2010-2014, MS-Cheminformatics LLC, All rights reserved." );
    }
    if ( auto article = doc_->append_child() ) {
        article.set_name( "article" );
    }
}

bool
document::save_file( const char * filepath ) const
{
    return doc_->save_file( filepath );
}

bool
document::load_file( const char * filepath )
{
    return doc_->load_file( filepath );
}

bool
document::save( std::ostream& o ) const
{
    doc_->save( o );
    return true;
}

bool
document::save( std::string& ar ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( ar );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
    doc_->save( device );
    return true;
}

bool
document::load( const char * xml )
{
    return doc_->load( xml );
}
