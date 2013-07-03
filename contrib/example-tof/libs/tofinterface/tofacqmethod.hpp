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

#ifndef tofACQUISITIONMETHOD_HPP
#define tofACQUISITIONMETHOD_HPP

#if defined WIN32
# include <cstdint>
#else
# include <tr1/cstdint>
#endif
#include <vector>

namespace tofinterface {

    class tofAcqMethod {
    public:
        class acqSegment {
        public:
            acqSegment();
            acqSegment( const acqSegment& );
			uint16_t startIndex() const;
            uint16_t numberOfAverage() const;
            uint32_t waitTime() const;
            uint16_t numberOfSamples() const;
            uint16_t stepOf() const;
            void startIndex( uint16_t );
            void numberOfAverage( uint16_t );
            void waitTime( uint32_t );
            void numberOfSamples( uint16_t );
            void stepOf( uint16_t );
        private:
            uint16_t startIndex_;
            uint16_t numberOfAverage_;
            uint16_t waitTime_;
            uint16_t numberOfSamples_;
            uint16_t stepOf_;
        };

        tofAcqMethod();
        tofAcqMethod( const tofAcqMethod& );
        uint32_t protocolId() const;
        void methodId( uint32_t );
        uint32_t methodId() const;
        uint16_t number_of_profiles() const;

        typedef acqSegment value_type;
        typedef std::vector< value_type > vector_type;

        operator const vector_type& () const;

    private:
        const uint32_t protocolId_;
        uint32_t methodId_;
        vector_type acqSegments_;
    };

}

#endif // tofACQUISITIONMETHOD_HPP
