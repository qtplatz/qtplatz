/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/counting/basic_histogram.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <memory>
#include <vector>
#include <cstdint>
#include <ostream>
#include <compiler/pragma_warning.hpp>

namespace aqmd3controls {

    class waveform;
    class AQMD3CONTROLSSHARED_EXPORT pkd_result;

    class pkd_result
        : public adportable::counting::basic_histogram< aqmd3controls::waveform::value_type
                                                        , aqmd3controls::waveform::meta_type
                                                        , adportable::counting::threshold_index > {

        pkd_result( const pkd_result& t ) = delete;
    public:
        pkd_result();
        pkd_result( std::shared_ptr< const waveform > d );

        std::shared_ptr< const waveform >& data();
        std::shared_ptr< const waveform > data() const;

    private:
        std::shared_ptr< const waveform > data_;
        std::pair< uint32_t, uint32_t > findRange_;
        uint32_t foundIndex_;
        bool findUp_;
    };

}
