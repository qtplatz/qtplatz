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

namespace boost { namespace uuids { struct uuid; } }

namespace adcontrols {
    namespace constants {

        // Folium (attachment) name
        const wchar_t * const F_DFT_FILTERD        = L"DFT Low Pass Filtered Spectrum";
        const wchar_t * const F_CENTROID_SPECTRUM  = L"Centroid Spectrum";
        const wchar_t * const F_MSPEAK_INFO        = L"MSPeakInfo";
        const wchar_t * const F_TARGETING          = L"Targeting";
        const wchar_t * const F_QUANSAMPLE         = L"QuanSample";
    }

    namespace iids {

        extern const boost::uuids::uuid massspectrometer_uuid;
        extern const boost::uuids::uuid adspectrometer_uuid;
    }

    enum hor_axis: unsigned int { hor_axis_mass, hor_axis_time };

}
