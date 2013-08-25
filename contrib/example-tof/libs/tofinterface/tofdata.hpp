/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef tofDATA_HPP
#define tofDATA_HPP

#include "tofstaticsetpts.hpp"
#include "tofstaticacts.hpp"
#include "tofacqmethod.hpp"
#include "cstdint.hpp"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
// #include <iostream>

namespace tofinterface {

    class tofDATA {
    public:
        class datum {
        public:
            datum();
            datum( const datum& );
            const tofAcqMethod::acqSegment& acqSegment() const;
            void acqSegment( const tofAcqMethod::acqSegment& );
            typedef int32_t value_type;
            typedef std::vector< value_type > vector_type;

            const vector_type& values() const;
            vector_type& values();

        private:
            tofAcqMethod::acqSegment acqSegment_;
            vector_type values_;

            friend class boost::serialization::access;
            template< class Archive >
            void serialize( Archive& ar, const unsigned int ) {
                ar & acqSegment_;
                ar & values_;
            }
        };

        tofDATA();
        tofDATA( const tofDATA& );
        uint32_t protocolId() const;

        uint32_t sequenceNumber() const;
        uint64_t rtcTimeStamp() const;
        uint64_t clockTimeStamp() const;
        uint32_t wellKnownEvents() const;
        uint32_t methodId() const;
        uint16_t numberOfProfiles() const;
        const tofStaticSetpts& setpts() const;
        const tofStaticActs& acts() const;
        const std::vector< datum >& data() const;	
        std::vector< datum >& data();

        void sequenceNumber( uint32_t );
        void rtcTimeStamp( uint64_t );
        void clockTimeStamp( uint64_t );
        void wellKnownEvents( uint32_t );
        void methodId( uint32_t );
        void numberOfProfiles( uint16_t );
        void setpts( const tofStaticSetpts& );
        void acts( const tofStaticActs& );
        void data( const std::vector< datum >& );

    private:
        uint32_t protocolId_;
        uint32_t sequenceNumber_;    // packet sequence number
        uint64_t rtcTimeStamp_;
        uint64_t clockTimeStamp_;
        uint32_t wellKnownEvents_;
        uint32_t methodId_;
        uint16_t numberOfProfiles_;
        tofStaticSetpts setpts_;
        tofStaticActs acts_;
        std::vector< datum > data_;

        friend class serializer;
        friend class boost::serialization::access;
        template< class Archive >
        void serialize( Archive& ar, const unsigned int ) {
            ar & protocolId_;
            ar & sequenceNumber_;    // packet sequence number
            ar & rtcTimeStamp_;
            ar & clockTimeStamp_;
            ar & wellKnownEvents_;
            ar & methodId_;
            ar & numberOfProfiles_;
            //ar & setpts_;
            //ar & acts_;
            ar & data_;
        }

    };

}

#endif // tofDATA_HPP
