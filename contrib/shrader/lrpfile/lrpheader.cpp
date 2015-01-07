/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "lrpheader.hpp"
#include <sstream>

namespace shrader {
    namespace detail {

#pragma pack(1)
        struct header {
            int32_t flags;
            char version[ 4 ];          // String 4 AAAD;
            int32_t type;               // Long 4 Data type code;
            char analdate[ 20 ];        // String 20 Analysis date;
            char analtime[ 8 ];         // String 8 Time of analysis;
            char instrument[40];        // String 40 Instrument description;
            char operator_name[ 40 ];   // String 40 Operator name;
            char calfile[ 8 ];          // String 8 Calibration file name;
            char library[ 8 ];          // String 8 Library name (selected ion monitoring only);
            char libcaldate[ 20 ];      // String 20 Last calibration date of library;
            int16_t interfacetype;      // integer 2 4=Jumbo; 5=MEV;
            int16_t rawdatatype;        // integer 2 0=not defined or not raw; 1=drive; 2=time;
            char SecondDmension[32];    // String 32;
            char dummy[ 28 ];           // String 28 Future space;
            int32_t AltTicPtr;          // Long 4 Pointer to Alternate TIC;
            int32_t nscans;             // Long 4 Number of scans in data file;
            int32_t setupptr;           // Long 4 Pointer to beginning of instrument set-up;
            int32_t calptr;             // Long 4 Pointer to beginning of calibration information;
            int32_t simptr;             // Long 4 Pointer to beginning of SIM set-up;
            int32_t scanptr;            // Long 4 Not used - Record of first spectrum (MSData);
            int32_t ticptr;             // Long 4 Pointer to first TIC master block;
            int32_t miscptr;            // Long 4 Pointer to beginning of misc. section;
            int32_t labelptr;           // Long 4 Pointer to beginning of scan labels;
        };
#pragma pack()
    }
}

using namespace shrader;

lrpheader::~lrpheader()
{
}

lrpheader::lrpheader(std::istream& in, size_t fsize) : loaded_( false )
{
    if ( fsize >= data_size ) {
        in.read( data_.data(), data_.size() );
        if ( !in.fail() )
            loaded_ = true;
    }
}

int32_t 
lrpheader::flags() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, flags ));
}

std::string 
lrpheader::version() const
{
    return std::string( data_.data() + offsetof( detail::header, version ), 4 );
}

int32_t 
lrpheader::type() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, type ));
}

std::string 
lrpheader::analdate() const
{
    return std::string( data_.data() + offsetof( detail::header, analdate ), 20 );
}

std::string 
lrpheader::analtime() const
{
    return std::string( data_.data() + offsetof( detail::header, analtime ), 8 );
}

std::string 
lrpheader::instrument() const
{
    return std::string( data_.data() + offsetof( detail::header, instrument ), 40 );
}

std::string 
lrpheader::operator_name() const
{
    return std::string( data_.data() + offsetof( detail::header, operator_name ), 40 );
}

std::string 
lrpheader::calfile() const
{
    return std::string( data_.data() + offsetof( detail::header, calfile ), 8 );
}

std::string 
lrpheader::library() const
{
    return std::string( data_.data() + offsetof( detail::header, library ), 8 );
}

std::string 
lrpheader::libcaldate() const
{
    return std::string( data_.data() + offsetof( detail::header, libcaldate ), 20 );
}

int16_t 
lrpheader::interfacetype() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::header, interfacetype ));
}

int16_t 
lrpheader::rawdatatype() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::header, rawdatatype ));
}

std::string 
lrpheader::SecondDmension() const
{
    return std::string( data_.data() + offsetof( detail::header, SecondDmension ), 32 );
}

size_t 
lrpheader::AltTicPtr() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, AltTicPtr ));
}

int32_t 
lrpheader::nscans() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, nscans ));
}

size_t 
lrpheader::setupptr() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, setupptr ));
}

size_t 
lrpheader::calptr() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, calptr ));
}

size_t 
lrpheader::simptr() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, simptr ));
}

int32_t 
lrpheader::scanptr() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, scanptr ));
}

size_t 
lrpheader::ticptr() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, ticptr ));
}

size_t 
lrpheader::miscptr() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, miscptr ));
}

size_t 
lrpheader::labelptr() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, labelptr ));
}

std::string
lrpheader::data_type_code() const
{
    std::ostringstream o;
    uint32_t code = type();
    o << ( code & 0x0001 ) ? "Mass Data" : "Raw Data";
    o << ", " << ( code & 0x0002 ) ? "Internal Mass Reference" : "External Mass Reference";
    o << ", " << ( code & 0x0004 ) ? "Bar data" : "Profile";
    o << ", " << (code & 0x8000) ? "Ion mobility data" : "";
    return o.str();
}

std::string
lrpheader::interfacetype_code() const
{
    std::ostringstream o;
    switch( interfacetype() ) {
    case 4: o << "GCmate"; break;
    case 5: o << "TSSWIN"; break;
    case 6: o << "EZScan"; break;
    case 7: o << "Translated Complement Files"; break;
    case 8: o << "Time Of Flight"; break;
    case 9: o << "Extrel(Merlin)"; break;
    }
    return o.str();
}

std::string
lrpheader::rawdatatype_code() const
{
    std::ostringstream o;

    switch( rawdatatype() ) {
    case 0: o << "undefined??"; break;
    case 1: o << "Drive Values"; break;
    case 2: o << "Time Values"; break;
    case 4: o << "Position Values"; break;
    }
    return o.str();
}
