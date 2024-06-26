// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
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
#include <accutofcontrols/constants.hpp>

namespace accutof {
    namespace spectrometer {

        // dataformat v2 support (dataformat v3 and later versions no longer using 'data interpreter' design)
        namespace dataInterpreter {
            struct spectrometer {
                static const wchar_t * name() { return L"AccuTOF"; }
            };
            struct traceObserver {
                static const wchar_t * name() { return L"AccuTOF::TimeTrace"; }
            };
        }

    }
}
