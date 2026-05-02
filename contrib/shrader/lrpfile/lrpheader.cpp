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
#include <adportable/debug.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <cstddef>
#include <sstream>
#include <boost/json.hpp>
#include <boost/algorithm/string.hpp>

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

lrpheader::lrpheader() : loaded_( false )
                         , data_{ 0 }
{
}

lrpheader::lrpheader( const lrpheader& t ) : loaded_( t.loaded_ )
                                           , data_{ t.data_ }
{
}

bool
lrpheader::load(std::istream& in, size_t fsize)
{
    if ( fsize >= data_size ) {
        in.read( data_.data(), data_.size() );
        if ( !in.fail() )
            loaded_ = true;
    }
    return loaded_;
}

int32_t
lrpheader::flags() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, flags ));
}

std::string
lrpheader::version() const
{
    auto a = std::string( data_.data() + offsetof( detail::header, version ), 4 );
    return boost::trim_copy( a );
}

int32_t
lrpheader::type() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::header, type ));
}

std::string
lrpheader::analdate() const
{
    auto a = std::string( data_.data() + offsetof( detail::header, analdate ), 20 );
    return boost::trim_copy( a );
}

std::string
lrpheader::analtime() const
{
    auto a = std::string( data_.data() + offsetof( detail::header, analtime ), 8 );
    return boost::trim_copy( a );
}

std::string
lrpheader::instrument() const
{
    auto a = std::string( data_.data() + offsetof( detail::header, instrument ), 40 );
    return boost::trim_copy_if(a, boost::is_any_of(" \0") );
}

std::string
lrpheader::operator_name() const
{
    auto a = std::string( data_.data() + offsetof( detail::header, operator_name ), 40 );
    return boost::trim_copy_if(a, boost::is_any_of(" \0") );
}

std::string
lrpheader::calfile() const
{
    auto a = std::string( data_.data() + offsetof( detail::header, calfile ), 8 );
    std::erase( a, '\0' );
    return boost::trim_copy( a );
}

std::string
lrpheader::library() const
{
    auto a = std::string( data_.data() + offsetof( detail::header, library ), 8 );
    std::erase( a, '\0' );
    return boost::trim_copy( a );
}

std::string
lrpheader::libcaldate() const
{
    auto a = std::string( data_.data() + offsetof( detail::header, libcaldate ), 20 );
    std::erase( a, '\0' );
    return boost::trim_copy( a );
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
    auto a = std::string( data_.data() + offsetof( detail::header, SecondDmension ), 32 );
    std::erase( a, '\0' );
    return boost::trim_copy(a);
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
    o <<  ( code & 0x0001 ? "Mass Data" : "Raw Data" );
    o << ", " << ( code & 0x0002 ? "Internal Mass Reference" : "External Mass Reference");
    o << ", " << ( code & 0x0004 ? "Bar data" : "Profile" );
    o << ", " << (code & 0x8000  ? "Ion mobility data" : "" );
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


namespace shrader {

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const lrpheader& t )
    {
        jv = {{ "header"
                    , {
                    { "flags",            t.flags() }
                    , { "version",        t.version() }
                    , { "type",           t.type() }
                    , { "analdate",        t.analdate()  }
                    , { "analtime",        t.analtime()  }
                    , { "instrument",      t.instrument()  }
                    , { "operator_name",   t.operator_name()  }
                    , { "calfile",         t.calfile()  }
                    , { "library",         t.library()  }
                    , { "libcaldate",      t.libcaldate()  }
                    , { "interfacetype",  t.interfacetype() }
                    , { "rawdatatype",    t.rawdatatype() }
                    , { "SecondDmension",  t.SecondDmension() }
                    //, { "dummy",      t.dummy }
                    , { "AltTicPtr",  t.AltTicPtr() }       // Long 4 Pointer to Alternate TIC;
                    , { "nscans",     t.nscans() }          // Long 4 Number of scans in data file;
                    , { "setupptr",   t.setupptr() }        // Long 4 Pointer to beginning of instrument set-up;
                    , { "calptr",     t.calptr() }          // Long 4 Pointer to beginning of calibration information;
                    , { "simptr",     t.simptr() }          // Long 4 Pointer to beginning of SIM set-up;
                    , { "scanptr",    t.scanptr() }         // Long 4 Not used - Record of first spectrum (MSData);
                    , { "ticptr",     t.ticptr() }          // Long 4 Pointer to first TIC master block;
                    , { "miscptr",    t.miscptr() }         // Long 4 Pointer to beginning of misc. section;
                    , { "labelptr",   t.labelptr() }        // Long 4 Pointer to beginning of scan labels;
                }
            }};
    }

    // lrpheader
    // tag_invoke( const boost::json::value_to_tag< lrpheader >&, const boost::json::value& jv )
    // {
    // }
}
