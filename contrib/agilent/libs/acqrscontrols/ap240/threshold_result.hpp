/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "../acqrscontrols_global.hpp"
#include "../threshold_result.hpp"
#include <adportable/counting/threshold_index.hpp>
#include <adportable/counting/counting_result.hpp>
#include <memory>
#include <vector>
#include <cstdint>
#include <ostream>

namespace adcontrols { class TimeDigitalHistogram; }

namespace acqrscontrols {

    namespace ap240 {
        class waveform;
    }

    typedef threshold_result_< ap240::waveform > ap240_threshold_result;
    ACQRSCONTROLSSHARED_EXPORT std::ostream& operator << ( std::ostream&, const ap240_threshold_result& );
}
