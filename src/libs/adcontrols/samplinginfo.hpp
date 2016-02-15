// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "adcontrols_global.h"
#include "metric/prefix.hpp"
#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/tuple/tuple.hpp>
#include "massspectrometer.hpp"

namespace adcontrols {

    template< typename T > class SamplingInfo_archive;

    class ADCONTROLSSHARED_EXPORT SamplingInfo {
    public:
        SamplingInfo();
        SamplingInfo( uint32_t sampInterval, uint32_t nDelay, uint32_t nCount, uint32_t nAvg, uint32_t mode );

        uint32_t sampInterval() const;  // ps
        void setSampInterval( uint32_t );
        
        uint32_t nSamplingDelay() const;
        void setNSamplingDelay( uint32_t );

        uint32_t nSamples() const;
        uint32_t mode() const;  // number of turns for InfiTOF, lenear|reflectron for MALDI etc

        void fSampInterval( double );
        double fSampInterval() const;
        double fSampDelay() const;
        void horPos( double );
        double horPos() const;
        void setDelayTime( double );
        double delayTime() const;
        size_t numberOfTriggers() const;
        void setNumberOfTriggers( size_t );
        
    private:
        uint32_t sampInterval_;  // ps
        uint32_t nSamplingDelay_;
        uint32_t nSamples_;
        uint32_t nAverage_;
        uint32_t mode_;  // number of turns for InfiTOF, lenear|reflectron for MALDI etc
        //--
        double fsampInterval_; // seconds
        double horPos_;        // seconds
        double delayTime_;     // digitizer delay time (seconds), this can be negative!
        
    private:
        friend class SamplingInfo_archive< SamplingInfo >;
        friend class SamplingInfo_archive< const SamplingInfo >;
        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
}

BOOST_CLASS_VERSION( adcontrols::SamplingInfo, 5 )

