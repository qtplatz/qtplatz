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
#include <boost/serialization/version.hpp>
#include <cstdint>

namespace boost { namespace serialization { class access; } }

namespace acqrscontrols {
    namespace u5303a {

        class ACQRSCONTROLSSHARED_EXPORT metadata {
        public:
            metadata();

            double initialXTimeSeconds;
            int64_t actualPoints;
            int32_t flags;
            int32_t actualAverages;
            int64_t actualRecords;
            double initialXOffset;
            double xIncrement;
            double scaleFactor;
            double scaleOffset;
            // CLASS VERSION 2
            int32_t dataType;  // 2(int16_t)|4(int32_t)|8(int64_t)|-8(double)
            // CLASS VERSION 3
            int32_t protocolIndex;

        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int version );
        };
    }
}

BOOST_CLASS_VERSION( acqrscontrols::u5303a::metadata, 3 )
