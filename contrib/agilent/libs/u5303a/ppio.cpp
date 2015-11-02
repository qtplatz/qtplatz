/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "ppio.hpp"

#if defined __linux
# include <unistd.h>
# include <sys/io.h>
# include <stdio.h>
#endif

ppio::ppio() : success_( false )
             , data_( 0 )
{
#if defined __linux
    success_ = ( iopl( 3 ) == 0 ) && ioperm( BASEPORT, 5, 1 ) == 0;
    if ( !success_ )
        perror( "ioperm" );
#endif
}

ppio::~ppio()
{
#if defined __linux
    if ( success_ )
        ioperm( BASEPORT, 5, 0 );
#endif
}

ppio::operator bool () const
{
    return success_;
}

ppio&
ppio::operator << ( const unsigned char d )
{
    data_ = d;
#if defined __linux
    if ( success_ )    
        outb( data_, BASEPORT + 0 );
#endif
    return *this;
}

ppio&
ppio::operator |= ( const uint8_t d )
{
    data_ |= d;
#if defined __linux
    if ( success_ )        
        outb( data_, BASEPORT + 0 );
#endif    
}

ppio&
ppio::operator &= ( const uint8_t d )
{
    data_ &= d;
#if defined __linux
    if ( success_ )            
        outb( data_, BASEPORT + 0 );
#endif        
}

uint8_t
ppio::data() const
{
    return data_;
}
