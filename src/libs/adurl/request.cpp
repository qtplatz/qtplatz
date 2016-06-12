// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "request.hpp"

using namespace adurl;

static inline unsigned char hex( unsigned char x )
{
    return x + ( x > 9 ? ( 'A' - 10 ) : '0' );
}

request::request()
{
}

std::string
request::url_encode( const std::string& in )
{
    std::ostringstream os;

    for ( std::string::const_iterator it = in.begin(); it != in.end(); ++it )   {
        if ( (*it >= 'a' && *it <= 'z') ||
             (*it >= 'A' && *it <= 'Z') ||
             (*it >= '0' && *it <= '9') )  {
            os << *it;
        } else if ( *it == ' ' ) {
            os << '+';
        } else {
            os << '%' << hex( *it >> 4 ) << hex( *it % 16 );
        }
    }
    return os.str();
}

