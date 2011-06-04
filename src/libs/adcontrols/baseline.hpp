// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#include "adcontrols_global.h"

#include "timeutil.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
//#include <boost/serialization/string.hpp>
//#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT Baseline {
    public:
        virtual ~Baseline();
        Baseline();
        Baseline( const Baseline& );
        
        long baseId() const;
        void baseId( long );

        long startPos() const;
        void StartPos( long );
       
        long StopPos() const;
        void StopPos( long );

        bool isManuallyModified() const;
        void manuallyModified( bool );

        double startHeight() const;
        double stopHeight() const;
        seconds_t   startTime() const;
        seconds_t   stopTime() const;
    
        void startHeight( double );
        void stopHeight( double );
        void startTime( const seconds_t& );
        void stopTime( const seconds_t& );

        double height(int pos) const;
    private:
        bool manuallyModified_;
        long baseId_;
        long startPos_;
        long stopPos_;
        double startHeight_;
        double stopHeight_;
        seconds_t startTime_;
        seconds_t stopTime_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP( manuallyModified_ );
                ar & BOOST_SERIALIZATION_NVP( baseId_ );
                ar & BOOST_SERIALIZATION_NVP( startPos_ );
                ar & BOOST_SERIALIZATION_NVP( stopPos_ );
                ar & BOOST_SERIALIZATION_NVP( startHeight_ );
                ar & BOOST_SERIALIZATION_NVP( stopHeight_ );
                ar & BOOST_SERIALIZATION_NVP( startTime_ );
                ar & BOOST_SERIALIZATION_NVP( stopTime_ );
            }
        }

    };

}


