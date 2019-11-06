/**************************************************************************
** Copyright (C) 2019-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <boost/uuid/uuid.hpp>

namespace aqmd3 {

    constexpr const char * const waveform_observer_name    = "1.aqmd3.ms-cheminfo.com";
    // 99075cc1-16fb-4afc-9f93-01280dd19fe9
    constexpr boost::uuids::uuid waveform_observer         = {{ 0x99, 0x07, 0x5C, 0xC1, 0x16, 0xFB, 0x4A, 0xFC, 0x9F, 0x93, 0x01, 0x28, 0x0D, 0xD1, 0x9F, 0xE9 }};
    constexpr const char * const waveform_datainterpreter  = "86a34e56-66ec-4bf2-89fe-1339bb9a0ba9";

    // internal purpose
    // 3ec803a9-35a4-45b9-b70f-8389d46ac118
    constexpr boost::uuids::uuid avrg_waveform_observer    = {{ 0x3e, 0xc8, 0x03, 0xa9, 0x35, 0xa4, 0x45, 0xb9, 0xb7, 0x0f, 0x83, 0x89, 0xd4, 0x6a, 0xc1, 0x18 }};

    // b3600237-527b-4689-8b25-4ca1c30b99dd
    constexpr const char * const histogram_observer_name   = "histogram.tdc.1.aqmd3.ms-cheminfo.com";
    // a897c9e6-fd92-4c14-a2b5-dc77d3584cc2
    constexpr boost::uuids::uuid histogram_observer        = {{ 0xa8, 0x97, 0xc9, 0xe6, 0xfd, 0x92, 0x4c, 0x14, 0xa2, 0xb5, 0xdc, 0x77, 0xd3, 0x58, 0x4c, 0xc2 }};
    constexpr const char * const histogram_datainterpreter = "697ab154-c178-481b-9c63-60d5c09e4f6c";

    constexpr const char * const tdc_observer_name         = "tdc.1.aqmd3.ms-cheminfo.com";
    // 7a67d368-5308-4774-ab9a-1782b13acacc
    constexpr boost::uuids::uuid tdc_observer              = {{ 0x7a, 0x67, 0xd3, 0x68, 0x53, 0x08, 0x47, 0x74, 0xab, 0x9a, 0x17, 0x82, 0xb1, 0x3a, 0xca, 0xcc }};
    constexpr const char * const tdc_datainterpreter       = "192294b2-b5f7-4b15-a7de-a1f2423efdc0";

    constexpr const char * const pkd_observer_name         = "pkd.1.aqmd3.ms-cheminfo.com";
    // 2753b9f1-b8f1-40c5-a270-dbe07239b942
    constexpr boost::uuids::uuid pkd_observer              = {{ 0x27, 0x53, 0xb9, 0xf1, 0xb8, 0xf1, 0x40, 0xc5, 0xa2, 0x70, 0xdb, 0xe0, 0x72, 0x39, 0xb9, 0x42 }};

    constexpr const char * const softavgr_observer_name    = "avgr.waveform.1.aqmd3.ms-cheminfo.com";
    // f589ea65-1c99-4abc-b961-9d6ed2cf10e7
    constexpr boost::uuids::uuid softavgr_observer         = {{ 0xf5, 0x89, 0xea, 0x65, 0x1c, 0x99, 0x4a, 0xbc, 0xb9, 0x61, 0x9d, 0x6e, 0xd2, 0xcf, 0x10, 0xe7 }};
    constexpr const char * const softavgr_datainterpreter  = "d252d006-556b-4827-afcd-72bee874860f";

    enum { nchannels = 2 }; // channel #2 reserved for peak detection in fpga
    enum SpectrumType { Profile, ProfileAvgd, ProfileLongTerm, Histogram, HistogramLongTerm, PkdWaveformLongTerm };

}
