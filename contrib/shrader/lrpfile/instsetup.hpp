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

#pragma once

#include <cstdint>
#include <string>
#include <array>

namespace shrader {

    class instsetup {
        enum {  data_size = 256 };
    public:
        ~instsetup();
        instsetup( std::istream& in, size_t fsize );
        inline operator bool () const { return loaded_; }

        int32_t flags() const;		 // Long 4 Record type code = 2
        int32_t ionization() const;	 // Long 4 Ionization method code
        int32_t upperdrive() const;	 // Long 4 Upper mass drive
        int32_t lowerdrive() const;	 // Long 4 Lower mass drive
        int32_t umasslim() const;	 // Long 4 Upper mass limit of scan * 65536
        int32_t lmasslim() const;	 // Long 4 Lower mass limit of scan * 65536
        int32_t ucallim() const;	 // Long 4 Upper mass limit of calibration * 65536
        int32_t lcallim() const;	 // Long 4 Lower mass limit of calibration * 65536
        int32_t aves() const;		 // Long 4 Number A/D readings per D/A step
        int32_t stepsize() const;	 // Long 4 Step size between data points
        float scanspeed() const;	 // Single 4 Scans/second (1/scantime)
        float scancycle() const;	 // Single 4 Interscan delay (msec)
        int32_t caltable() const;	 // Long 4 Calibration table used ? (0 or 1)
        int32_t scanmode() const;	 // Long 4 Scanning field code
        int32_t scanlaw() const;	 // Long 4 Scan law code
        int32_t resolution() const;	 // Long 4 Instrument resolution
        float reswindow() const;	 //  Single 4 Peak width used for peak detection
        float calslope() const;         // Single 4 Calibration slope (linear scan only)
        float calinter() const;         // Single 4 Calibration intercept (linear scan only)
        float clockbaud() const;		// Single 4 Clock baud rate in seconds / data point
        int32_t overload() const;		// Long 4 Maximum intensity (A/D max. - baseline value)
        int32_t timewindow() const;		// Long 4 not used
        float masswindow() const;		// Single 4 Mass window for selected ion monitoring
        float inttime() const;          // Single 4 Integration time for selected ion monitoring
        std::string method() const;		// String 8 Method name
        std::string autosamproc() const;	// String 8 Autosampler procedure name
        std::string gcproc() const;             // String 8 GC procedure name
        double TOFDrift() const;		 // Double 8 TOF correction factor
        float samplesize() const;		 // Single 4 Sample size
        std::string sampleunits() const; // String 16 Sample size units
        int16_t peakcentroid() const;	 // Integer 2 Centroiding method
        int16_t pkintensity() const;	 // Integer 2 Intensity method (height = 0, area = 1)
        float inithreshold() const;		 // Single 4 Threshold at low mass (as A/D value)
        float fnlthreshold() const;		 // Single 4 Threshold at high mass (as A/D value)
        double HVolt() const;           // Double 8 Accelerating voltage
        double HVscanBValue() const;	 // Double 8 Calibration intercept for HV scan
        float Peakthres() const;		// Single 4 Centroiding algorithm threshold (%)
        float baseline() const;         // Single 4 Measured instrument baseline
        float noise() const;            // Single 4 Measured instrument baseline noise
        float linkcorrection() const;	 // Single 4 Mass correction for linked scans
        float valley() const;           // Single 4 Centroiding algorithm valley (%)
        float minpeakwidth() const;		 // Single 4 Centroiding algorithm minimum peak width (%)
        int16_t sampletype() const;		 // Integer 2 0=Solid, 1=Solid by Dry Weight, 2=Liquid, 3=Gas
        int16_t unitscode() const;		 // Integer 2 Sample size units (0=ug, 3=Kg)
        int16_t dryweight() const;		 // Integer 2 Percent dry weight
        float linkmass() const;         // Single 4 Link mass
        int16_t SIMfield() const;		 // Integer 2 Switching Field for SIM
        int32_t SIMBset() const;		 // Long 4 Magnet Field reference value used for EF SIM
        double SIMBfield() const;		 // Double 8 Magnet field value used for EF SIM
        int32_t Slitcouple() const;		 // Integer 2 Silts coupled ?
        double Maxmassrange() const;	 // Double 8 Maximum mass range for instrument
        int32_t HvscanBDrive() const;	 // Long 4 Magnet field reference used for HV scan
        int16_t SIMCalOK() const;		 // Integer 2 SIM calibration OK
        float Maxvolt() const;          // Single 4 Maximum High voltage for instrument
        int16_t PeakFilter() const;		 // Integer 2
        int16_t TwoWayScan() const;		 // Integer 2
        std::string Dummy() const;		 // String 8 Future use

        std::string describe_ionization() const;
        std::string describe_scanmode() const;
        std::string describe_scanlaw() const;
        std::string describe_peakcentroid() const;

    private:
        std::array< char, data_size > data_;
        bool loaded_;
    };

}

