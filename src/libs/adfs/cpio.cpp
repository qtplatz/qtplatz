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

#include "cpio.hpp"

using namespace adfs;

static const size_t unit_size = 1024 * 64;

std::streamsize
detail::cpio::xsputn( const char_t * s, std::streamsize num )
{
    if ( count_ + num >= size_ )
        resize( size_t( num ) );
    for ( int i = 0; i < num; ++i )
        p_[ count_++ ] = *s++;
    return num;
}

//std::char_traits<char_t>::int_type
std::basic_streambuf<char_t>::int_type
detail::cpio::overflow ( int_type c )
{
    if ( count_ >= size_ )
        resize( sizeof( int_type )  );
    p_[ count_++ ] = c;
    return c;
}

bool
detail::cpio::resize( size_t num )
{
    size_ += unit_size * ( 1 + ( num / unit_size ) );
    char_t * pNew = new char_t[ size_ ];
    memcpy( pNew, p_.get(), count_ );
    p_.reset( pNew );
    return true;
}

////

detail::cpio::cpio( size_t size, char_t * p ) : size_(size), count_(size)
{
    if ( p ) {
        setg( p, p, p + size );
    } else {
        p_.reset( new char_t [ size ] );
        memset( p_.get(), 0, size * sizeof(char_t) );
        setg( p_.get(), p_.get(), p_.get() + size );
    }
}

std::basic_streambuf<char_t>::int_type
detail::cpio::underflow()
{
    return *gptr();
}

