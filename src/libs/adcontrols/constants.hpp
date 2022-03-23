// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#pragma once

#include "adcontrols_global.h"

namespace boost { namespace uuids { struct uuid; } }

namespace adcontrols {
    namespace constants {

        // Folium (attachment) name
        extern const wchar_t * const F_DFT_FILTERD;       //  = L"DFT Low Pass Filtered Spectrum";
        extern const wchar_t * const F_CENTROID_SPECTRUM; //  = L"Centroid Spectrum";
        extern const wchar_t * const F_MSPEAK_INFO;       //        = L"MSPeakInfo";
        extern const wchar_t * const F_TARGETING;                //           = L"Targeting";
        extern const wchar_t * const F_QUANSAMPLE;        //         = L"QuanSample";
        extern const wchar_t * const F_PROFILED_HISTOGRAM; // = L"Profiled Histogram";
        extern const wchar_t * const F_PEAKRESULT;        //          = L"PeakResult";

        // (attachment) dataType
        extern const wchar_t * const DT_PEAKRESULT;       //        = L"PeakResult";
    }

    namespace iids {

        extern const boost::uuids::uuid massspectrometer_uuid;

        ADCONTROLSSHARED_EXPORT
        extern const boost::uuids::uuid adspectrometer_uuid;
    }

    enum hor_axis: unsigned int { hor_axis_mass, hor_axis_time };

    enum ion_polarity: unsigned int { polarity_positive, polarity_negative };

    namespace plot {
        enum axis : unsigned int { yAxis, xAxis };
        enum unit : unsigned int { Arbitrary, Counts, Volts, AU, RIU };
    }

    extern const boost::uuids::uuid adcontrols_uuid;

    namespace Quan {
        enum QuanInlet {
            Chromatography
            , Infusion
            , Counting
            , ExportData // 2020-03-14, for mapping data export
        };
    }

}
