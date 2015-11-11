/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include "../acqrscontrols_global.hpp"
#include "device_method.hpp"
#include <adcontrols/threshold_method.hpp>
#include <adcontrols/threshold_action.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>
#include <iostream>

namespace boost { namespace serialization { class access; } }

namespace acqrscontrols {
    namespace u5303a {

        class ACQRSCONTROLSSHARED_EXPORT method {
        public:
            method();
            method( const method& t );
            static const char * modelClass() { return "u5303a"; }

            uint32_t channels_;
            uint32_t mode_;  // 0 := digitizer, 2 := averager
            device_method method_;
            adcontrols::threshold_method threshold_;
            adcontrols::threshold_action action_;

            static bool archive( std::ostream&, const method& );
            static bool restore( std::istream&, method& );
            static bool xml_archive( std::wostream&, const method& );
            static bool xml_restore( std::wistream&, method& );

        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
        };

    }
}

BOOST_CLASS_VERSION( acqrscontrols::u5303a::method, 5 )
