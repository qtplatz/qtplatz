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

#include "spchdr.hpp"
#include "spcfile.hpp"
#include <ctime>
#include <exception>

#if defined _MSC_VER
# pragma warning(disable:4800) // int forcing value to bool
#endif

using namespace galactic;

spchdr::spchdr( const SPCHDR * p ) : p_( p )
{
}

uint8_t
spchdr::ftflgs() const
{
    return p_->ftflgs; /* Flag bits defined below */
} 

// file data type flags
bool
spchdr::isMultiFileFormat() const
{
    return p_->ftflgs & TMULTI;
}

bool
spchdr::isRandomZValues() const
{
    return p_->ftflgs & TMULTI && p_->ftflgs & TRANDM;
}

bool
spchdr::isOrderedZValues() const
{
    return p_->ftflgs & TMULTI && p_->ftflgs & TORDRD;
}

bool
spchdr::hasAxisLabel() const
{
    return p_->ftflgs & TALABS;
}

bool
spchdr::hasUniqueXArray() const
{
    return p_->ftflgs & TXYXYS;
}

bool
spchdr::hasXArray() const
{
    return p_->ftflgs & TXVALS;
}

uint8_t
spchdr::fversn() const
{
    return p_->fversn; /* 0x4B=> new LSB 1st, 0x4C=> new MSB 1st, 0x4D=> old format */
} 

bool
spchdr::isDeprecated() const
{
    return p_->fversn == 0x4d;
}

bool
spchdr::isLittleEndian() const
{
    return ( p_->fversn == 0x4b ) ? true : ( p_->fversn == 0x4c ) ? false : throw std::exception();
}

uint8_t
spchdr::fexper() const
{
    return p_->fexper; /* Instrument technique code (see below) */
} 

int8_t
spchdr::fexp() const
{
    return p_->fexp; /* Fraction scaling exponent integer (80h=>float) */
}

uint32_t
spchdr::fnpts() const
{
    return p_->fnpts; /* Integer number of points (or TXYXYS directory position) */
}

double
spchdr::ffirst() const
{
    return p_->ffirst;   /* Floating X coordinate of first point */
}

double
spchdr::flast() const
{
    return p_->flast;	/* Floating X coordinate of last point */
}

uint32_t
spchdr::number_of_subfiles() const
{
    return p_->fnsub; /* Integer number of subfiles (1 if not TMULTI) */
}

uint8_t
spchdr::axis_type_x() const
{
    return p_->fxtype;  /* Type of X axis units (see definitions below) */
}

uint8_t
spchdr::axis_type_y() const
{
    return p_->fytype; /* Type of Y axis units (see definitions below) */
}

uint8_t
spchdr::axis_type_z() const
{ 
    return p_->fztype;  /* Type of Z axis units (see definitions below) */
}

uint8_t
spchdr::fpost() const
{
    return p_->fpost; /* Posting disposition (see GRAMSDDE.H) */
}

uint32_t
spchdr::fdate() const
{
    return p_->fdate;	/* Date/Time LSB: min=6b,hour=5b,day=5b,month=4b,year=12b */
}

const char *
spchdr::resolution_description() const
{
    return p_->fres; /* Resolution description text (null terminated) */
}

const char *
spchdr::source_instrument_description() const
{
    return p_->fsource;	/* Source instrument description text (null terminated) */
}

int16_t
spchdr::num_peak_points() const
{
    return p_->fpeakpt;	/* Peak point number for interferograms (0=not known) */
}

const char *
spchdr::comment() const
{
    return p_->fcmnt;	/* Null terminated comment ASCII text string */
}

const char *
spchdr::axis_label_text() const
{
    return p_->fcatxt;	/* X,Y,Z axis label strings if ftflgs=TALABS */
}

uint32_t
spchdr::logblock_offset() const
{
    return p_->flogoff; /* File offset to log block or 0 (see above) */
}

uint32_t
spchdr::file_modification_flags() const
{
    return p_->fmods;	/* File Modification Flags (see below: 1=A,2=B,4=C,8=D..) */
}

uint8_t
spchdr::processing_code() const
{
    return p_->fprocs;	/* Processing code (see GRAMSDDE.H) */
}

uint8_t
spchdr::calibration_level() const
{
    return p_->flevel; /* Calibration level plus one (1 = not calibration data) */
}

int16_t
spchdr::sub_method_sample_injection_number() const
{
    return p_->fsampin;	/* Sub-method sample injection number (1 = first or only ) */
}

float
spchdr::ffactor() const
{
    return p_->ffactor;	/* Floating data multiplier concentration factor (IEEE-32) */
}

const char *
spchdr::fmethod() const
{
    return p_->fmethod;	/* Method/program/data filename w/extensions comma list */
}

float
spchdr::fzinc() const
{
    return p_->fzinc;	/* Z subfile increment (0 = use 1st subnext-subfirst) */
}

uint32_t
spchdr::fwplanes() const
{
    return p_->fwplanes;	/* Number of planes for 4D with W dimension (0=normal) */
}

float
spchdr::fwinc() const
{
    return p_->fwinc;	/* W plane increment (only if fwplanes is not 0) */
}

uint8_t
spchdr::fwtype() const
{
    return p_->fwtype; /* Type of W axis units (see definitions below) */
}

const char *
spchdr::string_from_time( uint32_t d, std::string& sbuf )
{
    struct tm t;
    t.tm_sec = 0;
    t.tm_min = d & 0x3f;
    d >>= 6;
    t.tm_hour = d & 0x1f;
    d >>= 5;
    t.tm_mday = d & 0x1f;
    d >>= 5;
    t.tm_mon = ( d & 0x0f ) - 1;
    d >>= 4;
    t.tm_year = ( d & 0xfff ) - 1900;
    t.tm_isdst = 0;
    std::mktime( &t );
    sbuf = std::asctime( &t );
    std::replace( sbuf.begin(), sbuf.end(), '\n', '\0' );
    return sbuf.c_str();
}

void
spchdr::dump_spchdr( std::ostream& o ) const
{
    const SPCHDR& h = *p_;

    static const char * flags[] = { "TSPREC", "TCGRAM", "TMULTI", "TRANDM", "TORDRD", "TALABS", "TXYXYS", "TXVALS" };

    o << "================== SPCHDR ===================" << std::endl;
    o << std::hex << std::showbase
      << "ftflgs: " << int(h.ftflgs) << "\thas " << number_of_subfiles() << " subfiles\t";
    for ( unsigned i = 0; i < 8; ++i ) {
        if ( h.ftflgs & (1 << i) )
            o << flags[i] << ", ";
    }

    if ( isDeprecated() )
        o << "\tOld Data Format";
    else
        o << "\tEndian: " << ( isLittleEndian() ? "Little" : "Big" );
    o << std::endl;
    o << "Number of point: " << std::dec << std::showbase << fnpts();
    o << "\tacquisition range: " << ffirst() << " -- " << flast() << std::endl;

    static const char * experiments[] = {
        "General SPC (could be anything) "
        , "Gas Chromatogram "
        , "General Chromatogram (same as SPCGEN with TCGRAM) "
        , "HPLC Chromatogram "
        , "FT-IR, FT-NIR, FT-Raman Spectrum or Igram (Can also be used for scanning IR.) "
        , "NIR Spectrum (Usually multi-spectral data sets for calibration.) "
        , "UV-VIS Spectrum (Can be used for single scanning UV-VIS-NIR.) "
        , "X-ray Diffraction Spectrum "
        , "Mass Spectrum  (Can be single, GC-MS, Continuum, Centroid or TOF.) "
        , "NMR Spectrum or FID "
        , "Raman Spectrum (Usually Diode Array, CCD, etc. use SPCFTIR for FT-Raman.) "
        , "Fluorescence Spectrum "
        , "Atomic Spectrum "
        , "Chromatography Diode Array Spectra "
    };

    if ( fexper() < sizeof(experiments)/sizeof(experiments[0]) )
        o << "experiment:\t" << experiments[ fexper() ] << std::endl;

    std::string timestr;

    o << std::hex << std::showbase 
      << "Fraction scaling exponent integer:\t" << int( fexp() )
      << "\naxis_type(x,y,z):\t"  << spcfile::axis_type_x_string( axis_type_x() )
      << ", " << spcfile::axis_type_y_string( axis_type_y() )
      << ", " << spcfile::axis_type_x_string( axis_type_z() ) << std::endl
      << "\tPosting disposition:\t"                     << int(h.fpost)
      << "\nDate/Time:\t"                               << string_from_time( fdate(), timestr )
      << "\nResolution description:\t"                  << resolution_description()
      << "\nsource_instrument_description:\t"           << source_instrument_description()
      << "\nnum_peak_points:\t"                         << num_peak_points()
      << "\nComment:\t"                                 << comment();
    if ( ftflgs() & TALABS ) {
        o << "\naxis_label_text:\t"                         << axis_label_text();
    }
    if ( logblock_offset() ) {
        o << "\nFile offset to log block:\t"                << logblock_offset();
    }
    o << "\nFile Modification flags:\t"                 << file_modification_flags()
      << "\nProcessing code:\t"                         << int( processing_code() )
      << "\nCalibration level plus one:\t"              << int( calibration_level() )
      << "\nSub-method sample injection number:\t"      << sub_method_sample_injection_number()
      << "\nFloating data multiplier concentration factor:\t" << ffactor()
      << "\nMethod/program/data filename w/ extensions comma list:\t" << fmethod()
      << "\nZ subfile increment:\t"                     << fzinc()       
      << "\nNumber of pluns for 4D with W dimension:\t" << fwplanes()
      << "\nW plane increment:\t"                       << fwinc()    
      << "\nType of W axis units:\t"                    << int( fwtype() )
      << std::endl;
    o << "=================== end of SPCHDR ====================" << std::endl;
}
