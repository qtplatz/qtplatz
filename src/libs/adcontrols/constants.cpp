// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "adcontrols_global.h"
#include "constants.hpp"
#include <compiler/boost/workaround.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace adcontrols {

    namespace constants {

        const wchar_t * const F_DFT_FILTERD        = L"DFT Low Pass Filtered Spectrum";
        const wchar_t * const F_CENTROID_SPECTRUM  = L"Centroid Spectrum";
        const wchar_t * const F_MSPEAK_INFO        = L"MSPeakInfo";
        const wchar_t * const F_TARGETING          = L"Targeting";
        const wchar_t * const F_QUANSAMPLE         = L"QuanSample";
        const wchar_t * const F_PROFILED_HISTOGRAM = L"Profiled Histogram";
        const wchar_t * const F_PEAKRESULT         = L"Peak Result";

        // (attachment) dataType
        // const wchar_t * const DT_PEAKRESULT        = L"PeakResult";
    }

    namespace iids {

        const boost::uuids::uuid massspectrometer_uuid = boost::uuids::string_generator()( "{85897CCB-8025-41AB-B01E-3147C44A8955}" );

        // adspectrometer clsid (clsidSpectrometer column in ScanLaw table on the SQLite database)
        const boost::uuids::uuid adspectrometer_uuid = boost::uuids::string_generator()( "{E45D27E0-8478-414C-B33D-246F76CF62AD}" );
    }

    const boost::uuids::uuid adcontrols_uuid = boost::uuids::string_generator()( "{3D2F180E-18E9-43D3-9A37-9E981B509CAA}" );
}
