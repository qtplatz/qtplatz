// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "debug.hpp"
#include "debug_core.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "string.hpp"
#ifdef WIN32
#include <windows.h>
#endif
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace adportable;

debug::debug( const char * file, const int line ) : line_(line)
{
    if ( file )
        file_ = file;
}

debug::~debug(void)
{
    core::debug_core::instance()->log( 0, o_.str(), file_, line_ );
}

void
debug::initialize( const std::string& name )
{
    core::debug_core::instance()->open( name );
}

std::string
debug::where() const
{
    std::ostringstream o;
    if ( ! file_.empty() ) 
        o << file_ << "(" << line_ << "): ";
    return o.str();
}


template<> debug&
debug::operator << ( const std::wstring& text )
{
    o_ << adportable::string::convert( text );
    return *this;
}

template<> debug&
debug::operator << ( const boost::system::error_code& error )
{
    o_ << error.message();
    return *this;
}

template<> debug&
debug::operator << ( const boost::property_tree::ptree& pt )
{
    boost::property_tree::write_json( o_, pt );
    return *this;
}

debug&
debug::operator << ( const std::wstring& text )
{
    o_ << adportable::string::convert( text );
    return *this;
}

debug&
debug::operator << ( const wchar_t * text )
{
	o_ << adportable::string::convert( text );
	return *this;
}
