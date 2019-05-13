/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include <AcqirisDataTypes.h>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

namespace boost {
    namespace serialization {

        template<class Archive>
        void serialize( Archive& ar, AqDataDescriptor& t, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP(t.returnedSamplesPerSeg);
            ar & BOOST_SERIALIZATION_NVP(t.indexFirstPoint);
            ar & BOOST_SERIALIZATION_NVP(t.sampTime);
            ar & BOOST_SERIALIZATION_NVP(t.vGain);
            ar & BOOST_SERIALIZATION_NVP(t.vOffset);
            ar & BOOST_SERIALIZATION_NVP(t.returnedSegments);
            ar & BOOST_SERIALIZATION_NVP(t.nbrAvgWforms);
            ar & BOOST_SERIALIZATION_NVP(t.actualTriggersInAcqLo);
            ar & BOOST_SERIALIZATION_NVP(t.actualTriggersInAcqHi);
            ar & BOOST_SERIALIZATION_NVP(t.actualDataSize);
            ar & BOOST_SERIALIZATION_NVP(t.reserved2);
            ar & BOOST_SERIALIZATION_NVP(t.reserved3);
        }
        
        template<class Archive>
        void serialize( Archive& ar, AqSegmentDescriptorAvg& t, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP(t.horPos );
            ar & BOOST_SERIALIZATION_NVP(t.timeStampLo );
            ar & BOOST_SERIALIZATION_NVP(t.timeStampHi  );
            ar & BOOST_SERIALIZATION_NVP(t.actualTriggersInSeg );
            ar & BOOST_SERIALIZATION_NVP(t.avgOvfl );
            ar & BOOST_SERIALIZATION_NVP( t.avgStatus );
            ar & BOOST_SERIALIZATION_NVP(t.avgMax );
            ar & BOOST_SERIALIZATION_NVP(t.flags );
            ar & BOOST_SERIALIZATION_NVP(t.reserved );
        }

    }
}

namespace infitof {

    namespace acqiris {
        
        class AqDescriptors {
        public:
            AqDataDescriptor dataDesc;
            AqSegmentDescriptorAvg segDesc;
        private:
            friend class boost::serialization::access;
            template< class Archive >  
            void serialize( Archive& ar, const unsigned int ) {
                ar & BOOST_SERIALIZATION_NVP( dataDesc );
                ar & BOOST_SERIALIZATION_NVP( segDesc );
            }
        };
        
    }
}


