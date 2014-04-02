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

#include <spcfile.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <vector>
#include <iostream>


class spchdr {
public:
    spchdr( const galactic::SPCHDR * p ) : p_( p ) {}
    uint8_t ftflgs() const { return p_->ftflgs; } /* Flag bits defined below */
    // file data type flags
    bool isMultiFileFormat() const { return p_->ftflgs & TMULTI;   }
    bool isRandomZValues() const   { return p_->ftflgs & TMULTI && p_->ftflgs & TRANDM;   }
    bool isOrderedZValues() const  { return p_->ftflgs & TMULTI && p_->ftflgs & TORDRD;   }
    bool hasAxisLabel() const      { return p_->ftflgs & TALABS; }
    bool hasUniqueXArray() const   { return p_->ftflgs & TXYXYS; }
    bool hasXArray() const         { return p_->ftflgs & TXVALS; }

    uint8_t fversn() const { return p_->fversn; } /* 0x4B=> new LSB 1st, 0x4C=> new MSB 1st, 0x4D=> old format */
    bool isDeprecated() const { return p_->fversn == 0x4d; }
    bool isLittleEndian() const { return ( p_->fversn == 0x4b ) ? true : ( p_->fversn == 0x4c ) ? false : throw std::exception(); }

    uint8_t fexper() const { return p_->fexper; } /* Instrument technique code (see below) */
    int8_t  fexp() const  { return p_->fexp; }    /* Fraction scaling exponent integer (80h=>float) */
    uint32_t fnpts() const { return p_->fnpts; }  /* Integer number of points (or TXYXYS directory position) */
    double ffirst() const { return p_->ffirst; }  /* Floating X coordinate of first point */
    double flast() const { return p_->flast; }	/* Floating X coordinate of last point */
    uint32_t number_of_subfiles() const { return p_->fnsub;	} /* Integer number of subfiles (1 if not TMULTI) */
    uint8_t axis_type_x() const { return p_->fxtype; }  /* Type of X axis units (see definitions below) */
    uint8_t axis_type_y() const { return p_->fytype;	} /* Type of Y axis units (see definitions below) */
    uint8_t axis_type_z() const { return p_->fztype; }  /* Type of Z axis units (see definitions below) */
    uint8_t fpost() const { return p_->fpost; } /* Posting disposition (see GRAMSDDE.H) */
    uint32_t fdate() const { return p_->fdate; }	/* Date/Time LSB: min=6b,hour=5b,day=5b,month=4b,year=12b */
    const char * resolution_description() const { return p_->fres; } /* Resolution description text (null terminated) */
    const char * source_instrument_description() const { return p_->fsource; }	/* Source instrument description text (null terminated) */
    int16_t num_peak_points() const { return p_->fpeakpt; }	/* Peak point number for interferograms (0=not known) */
    // float  fspare[8];	/* Used for Array Basic storage */
    const char * comment() const { return p_->fcmnt; }	/* Null terminated comment ASCII text string */
    const char * axis_label_text() const { return p_->fcatxt; }	/* X,Y,Z axis label strings if ftflgs=TALABS */
    uint32_t logblock_offset() const { return p_->flogoff;	} /* File offset to log block or 0 (see above) */
    uint32_t file_modification_flags() const { return p_->fmods; }	/* File Modification Flags (see below: 1=A,2=B,4=C,8=D..) */
    uint8_t processing_code() const { return p_->fprocs; }	/* Processing code (see GRAMSDDE.H) */
    uint8_t calibration_level() const { return p_->flevel; } /* Calibration level plus one (1 = not calibration data) */
    int16_t sub_method_sample_injection_number() const { return p_->fsampin; }	/* Sub-method sample injection number (1 = first or only ) */
    float  ffactor() const { return p_->ffactor; }	/* Floating data multiplier concentration factor (IEEE-32) */
    const char * fmethod() const { return p_->fmethod; }	/* Method/program/data filename w/extensions comma list */
    float fzinc() const { return p_->fzinc; }	/* Z subfile increment (0 = use 1st subnext-subfirst) */
    uint32_t fwplanes() const { return p_->fwplanes; }	/* Number of planes for 4D with W dimension (0=normal) */
    float  fwinc() const { return p_->fwinc; }	/* W plane increment (only if fwplanes is not 0) */
    uint8_t fwtype() const { return p_->fwtype; } /* Type of W axis units (see definitions below) */
    // char   freserv[187]; /* Reserved (must be set to zero) */
    void dump();
private:
    const galactic::SPCHDR * p_;    
};

class subhdr {
public:
    subhdr( const galactic::SUBHDR * p ) : p_( p ) {}
    uint8_t subflgs() const { return p_->subflgs; }
    int8_t  subexp() const { return p_->subexp; }
    float time() const { return p_->subtime; }
    float time_next() const { return p_->subnext; }
    float noise() const { return p_->subnois; }                // peak pick noise level if high byte nonzero
    uint32_t number_of_samples() const { return p_->subnpts; }              // peak pick noise level if high byte nonzero
    uint32_t number_of_average() const { return p_->subscan; } // number of co-added scans or 0
    float wlevel() const { return p_->subwlevel; }             // W axis value (if fwplanes non-zero)
    void dump() const;
private:
    const galactic::SUBHDR * p_;
};

class spcfile {
public:
    spcfile( std::istream& in, size_t fsize ) : device_( fsize, 0 )
                                              , loaded_( false ) {
        in.read( device_.data(), fsize );
        if ( !in.fail() )
            loaded_ = true;
    }
    operator bool () const { return loaded_; }

    void dump();
    const galactic::SPCHDR * spchdr() const { return reinterpret_cast< const galactic::SPCHDR * >( device_.data() ); }
    const galactic::SUBHDR * subhdr() const { return reinterpret_cast< const galactic::SUBHDR * >( device_.data() + sizeof( galactic::SPCHDR ) ); }

    bool isDeprecated() const { return loaded_ && spchdr()->fversn == 0x4d; }
    bool isLittleEndian() const {
        return ( spchdr()->fversn == 0x4b ) ? true : ( spchdr()->fversn == 0x4c ) ? false : throw std::exception();
    }
    
private:
    std::vector< char > device_;
    bool loaded_;
};

void
spchdr::dump()
{
    const galactic::SPCHDR& h = *p_;

    std::cout << std::hex << std::showbase << "ftflgs: " << int(h.ftflgs) << std::endl;
    if ( h.ftflgs & TSPREC ) // 1
        std::cout << "Y data blocks are 16 bit integer (only if fexp if NOT 0x80)" << std::endl;

    if ( h.ftflgs & TCGRAM ) // 2
        std::cout << "Enables fexper in older software (not used)" << std::endl;

    if ( h.ftflgs & TMULTI ) // 4
        std::cout << "Multifile data format (more than one subfile)" << std::endl;

    if ( h.ftflgs & TRANDM ) // 8
        std::cout << "If TMULTI and TRANDM then Z values in SUBHDR structures are in random order (not used)" << std::endl;

    if ( h.ftflgs & TORDRD ) // 0x10
        std::cout << "If TMULTI and TORDRD then Z values are in ascending or descending ordered but not evenly spaced. Z values read from individual SUBHDR structures." << std::endl;

    if ( h.ftflgs & TALABS ) // 0x20
        std::cout << "Axis label text stored in fcatxt separated by nulls. Ignore fxtype, fytype, fztype corresponding to non-null text in fcatxt." << std::endl;

    if ( h.ftflgs & TXYXYS )  // 0x40
        std::cout << "Each subfile has unique X array; can only be used if TXVALS is also used. Used exclusively to flag as MS data for drawing as “sticks” rather than connected lines." << std::endl;

    if ( h.ftflgs & TXVALS ) // 0x80
        std::cout << "Non-evenly spaced X data. File has X value array preceding Y data block(s)." << std::endl;

    if ( isDeprecated() )
        std::cout << "Old Data Format";
    else
        std::cout << "Endian: " << ( isLittleEndian() ? "Little" : "Big" ) << std::endl;

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
        std::cout << experiments[ fexper() ] << std::endl;

    std::cout << std::hex << std::showbase 
              << "\nFraction scaling exponent integer:\t"             << int( fexp() )
              << "\nInteger number of point:\t"            << fnpts()
              << "\nFloating X coordinate of first point:\t"           << ffirst()
              << "\nFloating X coordinate of last point:\t"            << flast()
              << "\nNumber of sub files:\t"            << number_of_subfiles()
              << "\naxis_type_x:\t"           << int( axis_type_x() )
              << "\naxis_type_y:\t"           << int( axis_type_y() )
              << "\naxis_type_z:\t"           << int( axis_type_z() )
              << "\nPosting disposition:\t"   << int(h.fpost)
              << "\nDate/Time:\t"             << fdate()
              << "\nResolution description:\t"   << resolution_description()
              << "\nsource_instrument_description:\t" << source_instrument_description()
              << "\nnum_peak_points:\t"        << num_peak_points()
              << "\nComment:\t"                << comment()
              << "\naxis_label_text:\t"        << axis_label_text()
              << "\nFile offset to log block:\t"    << logblock_offset()
              << "\nFile Modification flags:\t"     << file_modification_flags()
              << "\nProcessing code:\t"           << int( processing_code() )
              << "\nCalibration level plus one:\t"           << int( calibration_level() )
              << "\nSub-method sample injection number:\t"   << sub_method_sample_injection_number()
              << "\nFloating data multiplier concentration factor:\t"          << ffactor()
              << "\nMethod/program/data filename w/ extensions comma list:\t"      << fmethod()
              << "\nZ subfile increment:\t"            << fzinc()       
              << "\nNumber of pluns for 4D with W dimension:\t" << fwplanes()
              << "\nW plane increment:\t"            << fwinc()    
              << "\nType of W axis units:\t"         << fwtype()     
              << std::endl;

}

void
subhdr::dump() const
{
    std::cout << "subflgs: " << subflgs() << std::endl;
    std::cout << "subexp: " << subexp() << std::endl;
    std::cout << "time: " << time() << std::endl;
    std::cout << "time_next: " << time_next() << std::endl;
    std::cout << "noise: " << noise() << std::endl;
    std::cout << "number_of_samples: " << number_of_samples() << std::endl;
    std::cout << "number_of_average: " << number_of_average() << std::endl;
    std::cout << "wlevel: " << wlevel() << std::endl;
}

void
spcfile::dump()
{
    if ( ! loaded_ )
        return;

    ::spchdr h( this->spchdr() );
    h.dump();

    ::subhdr subh( this->subhdr() );
    subh.dump();
}

void
dump( const char * file )
{
    boost::filesystem::path fpath( file );
    if ( boost::filesystem::exists( fpath ) ) {
        size_t fsize = boost::filesystem::file_size( fpath );
        boost::filesystem::ifstream in( fpath, std::ios_base::binary );
        spcfile d( in, fsize );
        d.dump();
        std::cout << "spcdump:\t" << fsize << " octets read from file " << fpath.string() << std::endl;
    }
}

int
main(int argc, char *argv[])
{
    while ( --argc ) {
        ++argv;
        dump( *argv );
    }
}


