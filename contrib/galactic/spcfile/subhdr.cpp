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

#include "subhdr.hpp"
#include "spc_h.hpp"
#include <iomanip>

using namespace galactic;

subhdr::subhdr( const SUBHDR * p ) : p_( p )
{
}

subhdr::subhdr( const subhdr& t ) : p_( t.p_ )
{
}

uint8_t
subhdr::subflgs() const
{
    return p_->subflgs;
}

int8_t
subhdr::subexp() const
{
    return p_->subexp;
}

int16_t
subhdr::subindx() const
{
    return p_->subindx;
}

float
subhdr::subtime() const
{
    return p_->subtime;
}

float
subhdr::subnext() const
{
    return p_->subnext;
}

float
subhdr::subnois() const
{ 
    return p_->subnois;                // peak pick noise level if high byte nonzero
}

uint32_t
subhdr::subnpts() const
{
    return p_->subnpts;              // peak pick noise level if high byte nonzero
}

uint32_t
subhdr::subscan() const
{
    return p_->subscan; // number of co-added scans or 0
}

float
subhdr::subwlevel() const
{
    return p_->subwlevel;             // W axis value (if fwplanes non-zero)
}

const uint8_t *
subhdr::data() const
{
    return reinterpret_cast< const uint8_t *>(p_) + sizeof( SUBHDR );
}

void
subhdr::dump_subhdr( std::ostream& o ) const
{
    o << "================== subhdr ===================" << std::endl;
    o << "subflgs: " << std::hex << std::showbase << int( subflgs() );
    if ( subflgs() & SUBCHGD )
        o << "\tSubflgs bit if subfile changed";
    if ( subflgs() & SUBNOPT )
        o << "\tSubflgs bit if peak table file should not be used";
    if ( subflgs() & SUBMODF )
        o << "\tSubflgs bit if subfile modified by arthmetic";
    o << std::endl;
    o << "Exponent for sub-files's Y values: " << int( subexp() ) << std::endl;
    o << "Integer index number of trace subfile (0=first): " << subindx() << std::endl;
    o << "Time for trace: " << std::fixed << std::setprecision(3) << subtime() << "\ttime for next trace: " << subnext() << std::endl;
    o << "Floating peak pick noise level if high byte nonzero: " << subnois() << std::endl;
    o << "number of subfile points for TXYXYS: " << int(subnpts()) << std::endl;
    o << "number of co-added scans: " << std::dec << int(subscan()) << std::endl;
    o << "subwlevel: " << subwlevel() << std::endl;
    o << "==================== end of subhdr ==================" << std::endl;
}
