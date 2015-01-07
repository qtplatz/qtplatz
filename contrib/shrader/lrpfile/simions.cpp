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

#include "simions.hpp"

namespace shrader {
    namespace detail {
#pragma pack(1)
        struct qlibdescription {
            char title[ 40 ];
            char master[ 8 ];
            char date[ 8 ];
            int16_t ncmpd;
            int16_t nis;
            char time[8];
            char version[2];
            int16_t units;
            char dummay[184];
        };

        struct CMASS {
            float mass;
            int16_t intensity;
        };

        struct qmasterllib {
            int32_t flags;// Long 4 Record type code = 4
            char compound[ 40 ]; // String 40 Compound name
            float dl; //dl Single 4 Detection limit
            float intmass; // Single 4 Primary integrating mass
            float rrt; // Single 4 Relative retention time
            int16_t pointer; // Integer 2 Pointer to internal standard
            int16_t naves; // Integer 2 Number of calibration points
            float stddev; // Single 4 Standard deviation of calibration points
            CMASS cmass[ 6 ]; // (6) User 36 Up to six confirming masses and intensities
            float a; // Single 4 Calibration intercept
            float b; // Single 4 Calibration slope
            float c; //  Single 4 Second order correction
            float d; // Single 4 Cubic correction
            float Q; // Single 4 Quality index
            float arearatio[8]; // Single 32 Area cmpd./area I.S. from a single measurement
            float amtratio[8];  // Single 32 Amt. compound/Amt I.S. added in that measurement
            char datafile[8][8]; // String*8 64 Data file used for this calibration
            int32_t qmethodcode; // Long 4 Quant. methods codes (specified by bit switches)
            char dummy[4]; // String 4 Future use
        };
#pragma pack()
    }
}

using namespace shrader;

simions::~simions()
{
}

simions::simions(std::istream& in, size_t fsize) : loaded_( false )
{
    auto pos = in.tellg();

    if ( ( fsize - in.tellg() ) >= data_size ) {
        in.read( data_.data(), data_.size() );
        if ( !in.fail() ) {
            if ( flags() == 38 ) { // TIC block
                in.seekg( pos ); // rewind
                return;
            }
            loaded_ = true;
        }
    }
}

int32_t 
simions::flags() const
{
    return *reinterpret_cast<const int32_t *>(data_.data() + offsetof( detail::qmasterllib, flags ));
}

