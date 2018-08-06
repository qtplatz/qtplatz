// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2018 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@qtplatz.com
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

#include "rapidjson_json.hpp"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

rapidjson_json::rapidjson_json() : doc( std::make_unique< rapidjson::Document >() )
{
}

rapidjson_json::~rapidjson_json()
{
}

bool
rapidjson_json::parse( const std::string& json_string )
{
    auto copystr( json_string );
    doc->ParseInsitu( const_cast< char * >( json_string.data() ) );
    return true;
}

std::string
rapidjson_json::stringify() const
{
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer< rapidjson::StringBuffer > writer( buffer );
    doc->Accept( writer );
    return buffer.GetString();
}

bool
rapidjson_json::map( data& d )
{
    return true;
}

std::string
rapidjson_json::make_json( const data& )
{
    return "";
}


