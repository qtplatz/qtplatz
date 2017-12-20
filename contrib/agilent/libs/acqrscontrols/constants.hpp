/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

namespace acqrscontrols {

    enum class Digitizer : unsigned int { DigitizerU5303A, DigitizerAP240 };

    namespace u5303a {
        const char * const waveform_observer_name        = "1.u5303a.ms-cheminfo.com";
        const char * const timecount_observer_name       = "timecount.1.u5303a.ms-cheminfo.com";           // 57a9eb79-a6a7-5e83-a7e9-2855f8cfb7e6
        const char * const histogram_observer_name       = "histogram.timecount.1.u5303a.ms-cheminfo.com"; // db7cbb6b-cf0b-582e-9f8f-89019265271b
        const char * const softavgr_observer_name        = "tdcdoc.waveform.1.u5303a.ms-cheminfo.com";     // f26ed645-7e3f-508e-b880-83812dc7594c
        const char * const tdcdoc_traces_observer_name   = "tdcdoc.traces.1.u5303a.ms-cheminfo.com";
        
        const char * const timecount_datainterpreter     = "timecount.u5303a.ms-cheminfo.com";
        const char * const histogram_datainterpreter     = "histogram.timecount.u5303a.ms-cheminfo.com";
        const char * const softavgr_datainterpreter      = "tdcdoc.waveform.u5303a.ms-cheminfo.com";
        const char * const tdcdoc_traces_datainterpreter = "tdcdoc.traces.u5303a.ms-cheminfo.com";        

        constexpr boost::uuids::uuid waveform_observer   = {{ 0xda, 0x16, 0x70, 0x4b, 0x2d, 0x18, 0x5c, 0x91, 0x82, 0xc7, 0x42, 0xa0, 0xa8, 0xb9, 0x4d, 0x2b }}; // u5303a_observer
        constexpr boost::uuids::uuid timecount_observer  = {{ 0x57, 0xa9, 0xeb, 0x79, 0xa6, 0xa7, 0x5e, 0x83, 0xa7, 0xe9, 0x28, 0x55, 0xf8, 0xcf, 0xb7, 0xe6 }};
        constexpr boost::uuids::uuid softavgr_observer   = {{ 0xf2, 0x6e, 0xd6, 0x45, 0x7e, 0x3f, 0x50, 0x8e, 0xb8, 0x80, 0x83, 0x81, 0x2d, 0xc7, 0x59, 0x4c }};
        constexpr boost::uuids::uuid histogram_observer  = {{ 0xdb, 0x7c, 0xbb, 0x6b, 0xcf, 0x0b, 0x58, 0x2e, 0x9f, 0x8f, 0x89, 0x01, 0x92, 0x65, 0x27, 0x1b }};

        // pkd_observer is a part (ch1) of pkd+avg -- this is the unique id for data-stream on .adfs file
        const char * const pkd_observer_name             = "pkd.1.u5303a.ms-cheminfo.com";
        constexpr boost::uuids::uuid pkd_observer        = {{ 0x22, 0xbe, 0xe4, 0x98, 0x70, 0x47, 0x4e, 0x55, 0x8e, 0x83, 0x32, 0x3f, 0x0d, 0xe9, 0xda, 0x16 }}; // u5303a pkd

        //constexpr boost::uuids::uuid trace_observer     = { 0x7f, 0xba, 0x8b, 0x2f, 0x94, 0x87, 0x5b, 0x7a, 0x89, 0xc2, 0xff, 0x94, 0xa5, 0x28, 0xbb, 0x77 };
        //7fba8b2f-9487-5b7a-89c2-ff94a528bb77	trace_observer
        //aac9c9a9-1191-5324-8178-db236eea1759	hv_dg_observer
        //522a83e8-b1b9-4341-8b0f-cac66d6d1e67	module_hv_httpd
        //34945677-4ad7-4d0f-b51d-4743f53514d4	module_dg_httpd
        //381d7d88-1bf5-4bfa-9d27-16e702b74640	module_protocol

        enum { nchannels = 2 };
    }

    namespace ap240 {
        const char * const waveform_observer_name        = "1.ap240.ms-cheminfo.com";
        const char * const timecount_observer_name       = "timecount.1.ap240.ms-cheminfo.com";           // 4f431f91-b08c-54ba-94f0-e1d13eba29d7
        const char * const histogram_observer_name       = "histogram.timecount.1.ap240.ms-cheminfo.com"; // 89a396e5-2f58-571a-8f0c-9da68dd31ae4
        const char * const softavgr_observer_name        = "tdcdoc.waveform.1.ap240.ms-cheminfo.com";     // eb9d5589-a3a4-582c-94c6-f7affbe8348a
        const char * const tdcdoc_traces_observer_name   = "tdcdoc.traces.1.ap240.ms-cheminfo.com";
        
        const char * const timecount_datainterpreter     = "timecount.ap240.ms-cheminfo.com";
        const char * const histogram_datainterpreter     = "histogram.timecount.ap240.ms-cheminfo.com";
        const char * const softavgr_datainterpreter      = "tdcdoc.waveform.ap240.ms-cheminfo.com";
        const char * const tdcdoc_traces_datainterpreter = "tdcdoc.traces.ap240.ms-cheminfo.com";        

        constexpr boost::uuids::uuid waveform_observer   = {{ 0x76, 0xd1, 0xf8, 0x23, 0x26, 0x80, 0x5d, 0xa7, 0x89, 0xf2, 0x4d, 0x2d, 0x95, 0x61, 0x49, 0xbd }}; //	ap240_observer
        constexpr boost::uuids::uuid timecount_observer  = {{ 0x4f, 0x43, 0x1f, 0x91, 0xb0, 0x8c, 0x54, 0xba, 0x94, 0xf0, 0xe1, 0xd1, 0x3e, 0xba, 0x29, 0xd7 }};
        constexpr boost::uuids::uuid softavgr_observer   = {{ 0x89, 0xa3, 0x96, 0xe5, 0x2f, 0x58, 0x57, 0x1a, 0x8f, 0x0c, 0x9d, 0xa6, 0x8d, 0xd3, 0x1a, 0xe4 }};
        constexpr boost::uuids::uuid histogram_observer  = {{ 0xeb, 0x9d, 0x55, 0x89, 0xa3, 0xa4, 0x58, 0x2c, 0x94, 0xc6, 0xf7, 0xaf, 0xfb, 0xe8, 0x34, 0x8a }};
        
        enum { nchannels = 2 };        
    }

    namespace dc122 {
        const char * const waveform_observer_name        = "1.dc122.ms-cheminfo.com";
        const char * const waveform_observer_uuid        = "{04c23c3c-7fd6-11e6-aa18-b7efcbc41dcd}";
        constexpr boost::uuids::uuid waveform_observer   = {{ 0x04, 0xc2, 0x3c, 0x3c, 0x7f, 0xd6, 0x11, 0xe6, 0xaa, 0x18, 0xb7, 0xef, 0xcb, 0xc4, 0x1d, 0xcd }};
        enum { nchannels = 1 };
    }
    
}
