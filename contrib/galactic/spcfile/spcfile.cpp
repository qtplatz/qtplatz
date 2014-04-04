/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@MS-Cheminformatics.com
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

#include "spcfile_global.hpp"
#include "spcfile.hpp"
#include "spc_h.hpp"
#include "spchdr.hpp"
#include "subhdr.hpp"
#include <istream>

//
using namespace galactic;

spcfile::spcfile( std::istream& in, size_t fsize ) : device_( fsize, 0 )
                                                   , spchdr_( 0 )
                                                   , loaded_( false ) {
    in.read( device_.data(), fsize );
    if ( !in.fail() ) {
        loaded_ = true;
        spchdr_ = new galactic::spchdr( reinterpret_cast< const SPCHDR * >( device_.data() ) );
        const char * psub = device_.data() + sizeof( SPCHDR );
        subhdrs_.push_back( std::make_shared< galactic::subhdr >( reinterpret_cast< const SUBHDR * >( psub ), spchdr_->p() ) );
        if ( spchdr_->isMultiFileFormat() ) {
            for ( size_t i = 1; i < spchdr_->number_of_subfiles(); ++i ) {
                psub += sizeof( SUBHDR ) + spchdr_->fnpts() * sizeof(uint32_t);
                subhdrs_.push_back( std::make_shared< galactic::subhdr >( reinterpret_cast< const SUBHDR * >( psub ), spchdr_->p() ) );
            }
        }

        spchdr::string_from_time( spchdr_->fdate(), date_ );
        if ( spchdr_->ftflgs() & TALABS ) {
			const char * p = spchdr_->axis_label_text();
			while ( p && *p && std::distance( spchdr_->axis_label_text(), p ) < 30 )
				axis_x_label_ += *p++;
			++p;
			while ( p && *p && std::distance( spchdr_->axis_label_text(), p ) < 30 )
				axis_y_label_ += *p++;
        }
    }
}

spcfile::~spcfile()
{
    delete spchdr_;
}

spcfile::operator bool () const
{
    return loaded_;
}

const spchdr *
spcfile::spchdr() const
{
    return spchdr_;
}

const subhdr *
spcfile::subhdr( size_t idx ) const
{
    if ( subhdrs_.size() > idx )
        return subhdrs_[ idx ].get();
    return 0;
}

size_t
spcfile::number_of_subfiles() const
{
    return subhdrs_.size();
}

const char *
spcfile::axis_x_label() const
{
    if ( axis_x_label_.empty() )
        return spchdr_->axis_type_x() == XARB ? "<i>m/z</i>" : axis_type_x_string( spchdr_->axis_type_x() );
    return axis_x_label_.c_str();
}

const char *
spcfile::axis_y_label() const
{
    if ( axis_y_label_.empty() )
        return spchdr_->axis_type_y() == YARB ? "Intensity" : axis_type_y_string( spchdr_->axis_type_y() );
    return axis_y_label_.c_str();
}

const char *
spcfile::source_instrument_description() const
{
    if ( spchdr_ )
        return spchdr_->source_instrument_description();
    return "no data";
}

bool
spcfile::isDeprecated() const
{
    return spchdr_ && spchdr_->isDeprecated();
}

bool
spcfile::isLittleEndian() const
{
    return spchdr_ && spchdr_->isLittleEndian();
}

const char *
spcfile::date() const
{
    return date_.c_str();
}

// static
const char *
spcfile::axis_type_x_string( int x )
{
    // x, z, w axis are here

    static const char * x_axis_types [] = {
        "Arbitrary"
        , "Wavenumber (cm-1)"
        , "Micrometers (um)"
        , "Nanometers (nm)"
        , "Seconds"
        , "Minutes"
        , "Hertz (Hz)"
        , "Kilohertz (KHz)"
        , "Megahertz (MHz)"
        , "Mass (M/z)"
        , "Parts per million (PPM)"
        , "Days"
        , "Years"
        , "Raman Shift (cm-1)"
        , "eV"
        , "XYZ text labels in fcatxt (old 0x4D version only)"
        , "Diode Number"
        , "Channel"
        , "Degrees"
        , "Temperature (F)"
        , "Temperature (C)"
        , "Temperature (K)"
        , "Data Points"
        , "Milliseconds (mSec)"
        , "Microseconds (uSec)"
        , "Nanoseconds (nSec)"
        , "Gigahertz (GHz)"
        , "Centimeters (cm)"
        , "Meters (m)"
        , "Millimeters (mm)"
        , "Hours"
    };
    if ( x < sizeof( x_axis_types ) / sizeof( x_axis_types[0] ) )
        return x_axis_types[ x ];
    if ( x == XDBLIGM )
        return "Double interferogram (no display labels)";
    return "Unknown";
}

const char *
spcfile::axis_type_y_string( int y )
{
    static const char * y_axis_types [] = {
        "Arbitrary Intensity"
        ,"Interferogram"
        ,"Absorbance"
        ,"Kubelka-Monk"
        ,"Counts"
        ,"Volts"
        ,"Degrees"
        ,"Milliamps"
        ,"Millimeters"
        ,"Millivolts"
        ,"Log(1/R)"
        ,"Percent"

        ,"Intensity"
        ,"Relative Intensity"
        ,"Energy"
        ,"Decibel"
        ,"Temperature (F)"
        ,"Temperature (C)"
        ,"Temperature (K)"
        ,"Index of Refraction [N]"
        ,"Extinction Coeff. [K]"
        ,"Real"
        ,"Imaginary"
        ,"Complex"
    };

    if ( y < sizeof( y_axis_types ) / sizeof( y_axis_types[0] ) )
        return y_axis_types[ y ];

    switch( y ) {
    case YTRANS:  return "Transmission (ALL HIGHER MUST HAVE VALLEYS!)";
    case YREFLEC: return "Reflectance";
    case YVALLEY: return "Arbitrary or Single Beam with Valley Peaks";
    case YEMISN:  return "Emission";
    }
    return "Unknown";
}
