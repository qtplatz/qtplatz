// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include <boost/uuid/uuid.hpp>

namespace accutofspectrometer {

    // dataformat v2 support (dataformat v3 and later versions no longer using 'data interpreter' design)
    namespace dataInterpreter {
        struct spectrometer {
            static const wchar_t * name() { return L"AccuTOF"; }
            static const char * utf8_name() { return "AccuTOF"; }
        };
        struct traceObserver {
            static const wchar_t * name() { return L"AccuTOF::TimeTrace"; }
        };
    }

    namespace names {
        const char * const iid_accutofspectrometer_plugin = "jp.ac.osaka-u.qtplatz.adplugins.massSpectrometer.accutof";
        const char * const objtext_massspectrometer = "AccuTOF"; // (don't change)
        const char * const objtext_datainterpreter = "AccuTOF"; // (don't change)
    }

    namespace iids {

        // 9568b15d-73b6-48ed-a1c7-ac56a308f712;
        constexpr boost::uuids::uuid uuid_massspectrometer = {{ 0x95, 0x68, 0xb1, 0x5d, 0x73, 0xb6, 0x48, 0xed, 0xa1, 0xc7, 0xac, 0x56, 0xa3, 0x08, 0xf7, 0x12 }};

        // 6472a93e-7f23-41ac-8e48-d288a9faeee8;
        const boost::uuids::uuid uuid_datainterpreter = {{ 0x64, 0x72, 0xa9, 0x3e, 0x7f, 0x23, 0x41, 0xac, 0x8e, 0x48, 0xd2, 0x88, 0xa9, 0xfa, 0xee, 0xe8 }};

    }
}
