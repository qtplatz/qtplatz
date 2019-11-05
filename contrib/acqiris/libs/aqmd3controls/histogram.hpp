/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "aqmd3controls_global.hpp"
#include "waveform.hpp"
#include <adportable/counting/threshold_index.hpp>
#include <adportable/basic_waveform.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <memory>
#include <vector>
#include <cstdint>
#include <ostream>
#include <compiler/pragma_warning.hpp>


namespace aqmd3controls {

    class waveform;

    class AQMD3CONTROLSSHARED_EXPORT histogram;

    class histogram : public adportable::basic_waveform< std::pair< uint32_t, int32_t >, aqmd3controls::meta_data > {
        histogram( const histogram& t ) = delete;
    public:
        histogram();
    };

}
