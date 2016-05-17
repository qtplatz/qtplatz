/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "pio.hpp"
#include <dgpio/dgpio_ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> // close
#include <sys/ioctl.h>

using namespace dgpio;

pio::~pio()
{
    close( fd_ );
}

pio::pio() : fd_( 0 )
{
}

bool
pio::open( const char * dev )
{
    return ( fd_ = ::open ( dev, O_RDONLY ) ) > 0;
}

int
pio::protocol_number() const
{
    uint8_t d( 0 );

    if ( fd_ > 0 && read( fd_, &d, sizeof( d ) ) == sizeof( d ) )
        return d;

    return -1;
}

bool
pio::set_protocol_number( uint8_t d )
{
    return ( fd_ > 0 && ioctl( fd_, DGPIO_SET_DATA, &d ) == 0 );
}
