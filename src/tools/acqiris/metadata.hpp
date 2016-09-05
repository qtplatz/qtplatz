/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <boost/serialization/version.hpp>
#include <cstdint>

namespace boost { namespace serialization { class access; } }

namespace acqiris {

    class metadata {
    public:
        metadata();
        int64_t actualPoints;
        int32_t flags;           // IO pin states
        int32_t actualAverages;  // 0 = digitizer data, 1..n averaged data
        int32_t indexFirstPoint; // firstValidPoint in U5303A
        int16_t channel;         // 1|2
        int16_t dataType;        // 1, 2, 4 := int8_t, int16_t, int32_t
        int32_t protocolIndex;            
        double initialXTimeSeconds;
        double initialXOffset;
        double xIncrement;
        double scaleFactor;
        double scaleOffset;
        double horPos;
    private:
        friend class boost::serialization::access;
        template<class Archive>  void serialize( Archive& ar, const unsigned int version );
    };
}

BOOST_CLASS_VERSION( acqiris::metadata, 0 )
