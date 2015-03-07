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

#include "instsetup.hpp"
#include <sstream>
#include <cstddef>

namespace shrader {
    namespace detail {
#pragma pack(1)
        struct instsetup {
            int32_t flags;      // Long 4 Record type code = 2
            int32_t ionization;	// Long 4 Ionization method code
            int32_t upperdrive;	// Long 4 Upper mass drive
            int32_t lowerdrive;	// Long 4 Lower mass drive
            int32_t umasslim;	// Long 4 Upper mass limit of scan * 65536
            int32_t lmasslim;	// Long 4 Lower mass limit of scan * 65536
            int32_t ucallim;	// Long 4 Upper mass limit of calibration * 65536
            int32_t lcallim;	// Long 4 Lower mass limit of calibration * 65536
            int32_t aves;       // Long 4 Number A/D readings per D/A step
            int32_t stepsize;	// Long 4 Step size between data points
            float scanspeed;	// Single 4 Scans/second (1/scantime)
            float scancycle;	// Single 4 Interscan delay (msec)
            int32_t caltable;	// Long 4 Calibration table used ? (0 or 1)
            int32_t scanmode;	// Long 4 Scanning field code
            int32_t scanlaw;	// Long 4 Scan law code
            int32_t resolution;	// Long 4 Instrument resolution
            float reswindow;	//  Single 4 Peak width used for peak detection
            float calslope;     // Single 4 Calibration slope (linear scan only)
            float calinter;     // Single 4 Calibration intercept (linear scan only)
            float clockbaud;	// Single 4 Clock baud rate in seconds / data point
            int32_t overload;	// Long 4 Maximum intensity (A/D max. - baseline value)
            int32_t timewindow;	// Long 4 not used
            float masswindow;	// Single 4 Mass window for selected ion monitoring
            float inttime;      // Single 4 Integration time for selected ion monitoring
            char  method[8];	// String 8 Method name
            char  autosamproc[8];	// String 8 Autosampler procedure name
            char  gcproc[8];	// String 8 GC procedure name
            double TOFDrift;	// Double 8 TOF correction factor
            float samplesize;	// Single 4 Sample size
            char  sampleunits[16];	// String 16 Sample size units
            int16_t peakcentroid;	// Integer 2 Centroiding method
            int16_t pkintensity;	// Integer 2 Intensity method (height = 0, area = 1)
            float inithreshold;	// Single 4 Threshold at low mass (as A/D value)
            float fnlthreshold;	// Single 4 Threshold at high mass (as A/D value)
            double HVolt;	// Double 8 Accelerating voltage
            double HVscanBValue;	// Double 8 Calibration intercept for HV scan
            float Peakthres;	// Single 4 Centroiding algorithm threshold (%)
            float baseline;	// Single 4 Measured instrument baseline
            float noise;	// Single 4 Measured instrument baseline noise
            float linkcorrection;	// Single 4 Mass correction for linked scans
            float valley;	// Single 4 Centroiding algorithm valley (%)
            float minpeakwidth;	// Single 4 Centroiding algorithm minimum peak width (%)
            int16_t sampletype;	// Integer 2 0=Solid, 1=Solid by Dry Weight, 2=Liquid, 3=Gas
            int16_t unitscode;	// Integer 2 Sample size units (0=ug, 3=Kg)
            int16_t dryweight;	// Integer 2 Percent dry weight
            float linkmass;	// Single 4 Link mass
            int16_t SIMfield;	// Integer 2 Switching Field for SIM
            int32_t SIMBset;	// Long 4 Magnet Field reference value used for EF SIM
            double SIMBfield;	// Double 8 Magnet field value used for EF SIM
            int32_t Slitcouple;	// Integer 2 Silts coupled ?
            double Maxmassrange;	// Double 8 Maximum mass range for instrument
            int32_t HvscanBDrive;	// Long 4 Magnet field reference used for HV scan
            int16_t SIMCalOK;	// Integer 2 SIM calibration OK
            float Maxvolt;	// Single 4 Maximum High voltage for instrument
            int16_t PeakFilter;	// Integer 2
            int16_t TwoWayScan;	// Integer 2
            char  Dummy[8];	// String 8 Future use
        };
#pragma pack()
    }
}

using namespace shrader;

instsetup::~instsetup()
{
}

instsetup::instsetup(std::istream& in, size_t fsize) : loaded_( false )
{
    if ( ( fsize - in.tellg() ) >= data_size ) {
        in.read( data_.data(), data_.size() );
        if ( !in.fail() )
            loaded_ = true;
    }
}

int32_t
instsetup::flags() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, flags ) );
}

int32_t
instsetup::ionization() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, ionization ) );
}

int32_t
instsetup::upperdrive() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, upperdrive ) );
}

int32_t
instsetup::lowerdrive() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, lowerdrive ));
}

int32_t
instsetup::umasslim() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, umasslim ) );
}

int32_t
instsetup::lmasslim() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, lmasslim ) );
}

int32_t
instsetup::ucallim() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, ucallim ) );
}

int32_t
instsetup::lcallim() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, lcallim ) );
}

int32_t
instsetup::aves() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, aves ) );
}

int32_t
instsetup::stepsize() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, stepsize ) );
}

float
instsetup::scanspeed() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, scanspeed ) );
}

float
instsetup::scancycle() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, scancycle ) );
}

int32_t
instsetup::caltable() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, caltable ) );
}

int32_t
instsetup::scanmode() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, scanmode ) );
}

int32_t
instsetup::scanlaw() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, scanlaw ) );
}

int32_t
instsetup::resolution() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, resolution ) );
}

float
instsetup::reswindow() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, reswindow ) );
}

float
instsetup::calslope() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, calslope ) );
}

float
instsetup::calinter() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, calinter ));
}

float
instsetup::clockbaud() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, clockbaud ) );
}

int32_t
instsetup::overload() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, overload ) );
}

int32_t
instsetup::timewindow() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, timewindow ) );
}

float
instsetup::masswindow() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, masswindow ) );
}

float
instsetup::inttime() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, inttime ) );
}

std::string
instsetup::method() const
{
    return std::string( data_.data() + offsetof( detail::instsetup, method ), 8 );
}

std::string
instsetup::autosamproc() const
{
    return std::string( data_.data() + offsetof( detail::instsetup, autosamproc ), 8 );
}

std::string
instsetup::gcproc() const
{
    return std::string( data_.data() + offsetof( detail::instsetup, gcproc ), 8 );
}

double
instsetup::TOFDrift() const
{
    return *reinterpret_cast<const double *>(data_.data() + offsetof( detail::instsetup, TOFDrift ) );
}

float
instsetup::samplesize() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, samplesize ) );
}

std::string
instsetup::sampleunits() const
{
    return std::string( data_.data() + offsetof( detail::instsetup, sampleunits ), 16 );
}

int16_t
instsetup::peakcentroid() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, peakcentroid ) );
}

int16_t
instsetup::pkintensity() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, pkintensity ) );
}

float
instsetup::inithreshold() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, inithreshold ) );
}

float
instsetup::fnlthreshold() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, fnlthreshold ) );
}

double
instsetup::HVolt() const
{
    return *reinterpret_cast<const double *>(data_.data() + offsetof( detail::instsetup, HVolt ) );
}

double
instsetup::HVscanBValue() const
{
    return *reinterpret_cast<const double *>(data_.data() + offsetof( detail::instsetup, HVscanBValue ) );
}

float
instsetup::Peakthres() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, Peakthres ) );
}

float
instsetup::baseline() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, baseline ) );
}

float
instsetup::noise() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, noise ) );
}

float
instsetup::linkcorrection() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, linkcorrection ) );
}

float
instsetup::valley() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, valley ) );
}

float
instsetup::minpeakwidth() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, minpeakwidth ) );
}

int16_t
instsetup::sampletype() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, sampletype ) );
}

int16_t
instsetup::unitscode() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, unitscode ) );
}

int16_t
instsetup::dryweight() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, dryweight ) );
}

float
instsetup::linkmass() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, linkmass ) );
}

int16_t
instsetup::SIMfield() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, SIMfield ) );
}

int32_t
instsetup::SIMBset() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, SIMBset ) );
}

double
instsetup::SIMBfield() const
{
    return *reinterpret_cast<const double *>(data_.data() + offsetof( detail::instsetup, SIMBfield ) );
}

int32_t
instsetup::Slitcouple() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, Slitcouple ) );
}

double
instsetup::Maxmassrange() const
{
    return *reinterpret_cast<const double *>(data_.data() + offsetof( detail::instsetup, Maxmassrange ) );
}

int32_t
instsetup::HvscanBDrive() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::instsetup, HvscanBDrive ) );}

int16_t
instsetup::SIMCalOK() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, SIMCalOK ) );
}

float
instsetup::Maxvolt() const
{
    return *reinterpret_cast<const float *>(data_.data() + offsetof( detail::instsetup, Maxvolt ) );
}

int16_t
instsetup::PeakFilter() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, PeakFilter ) );
}

int16_t
instsetup::TwoWayScan() const
{
    return *reinterpret_cast<const int16_t *>(data_.data() + offsetof( detail::instsetup, TwoWayScan ) );
}

std::string
instsetup::Dummy() const
{
    return std::string( data_.data() + offsetof( detail::instsetup, Dummy ), 8 );
}

std::string
instsetup::describe_ionization() const
{
    const int32_t code = ionization();
    std::ostringstream o;
    o << ( ( code & 0x0001 ) ? "Negative" : "Positive" );
    switch( code & 0x00fe ) {
    case 2: o << "Electron impact"; break;
    case 4: o << "Chemical ionization"; break;
    case 8: o << "FAB"; break;
    case 16: o << "Field desoprtion"; break;
    case 32: o << "Electrospray"; break;
    case 64: o << "Thermospray"; break;
    case 128: o << "Misc 1"; break;
    };
    o << ", ionization energy: " << ( ( code & 0x0000ff00 ) >> 8 );
    return o.str();
}

std::string
instsetup::describe_scanmode() const
{
    const int32_t code = scanmode();
    std::ostringstream o;
    switch( code & 0x003f ) {
    case 1: o << "Quad 1"; break;
    case 2: o << "Magnet 1"; break;
    case 4: o << "Accelerating Voltage"; break;
    case 8: o << "Quad 3"; break;
    case 16: o << "Magnet 2"; break;
    case 32: o << "Time of Flight"; break;
    } 
    return o.str();
}

std::string
instsetup::describe_scanlaw() const
{
    const int32_t code = scanlaw();
    std::ostringstream o;
    o << ( code & 0x0001 ) ? "Down" : "Up";
    switch( code & 0xfffe ) {
    case 2: o << ", Linear"; break;
    case 4: o << ", Inverse"; break;
    case 8: o << ", Exponential"; break;
    case 16: o << ", Quadratic"; break;
    case 32: o << ", Misc"; break;
    } 
    return o.str();
}

std::string
instsetup::describe_peakcentroid() const
{
    const int32_t code = peakcentroid();
    std::ostringstream o;
    switch( code ) {
    case 0: o << "Peak Top"; break;
    case 1: o << "Moments"; break;
    case 2: o << "Half Area"; break;
    case 3: o << "Center at 1/2 peak height"; break;
    case 4: o << "Hall Probe"; break;
    } 
    return o.str();
}


