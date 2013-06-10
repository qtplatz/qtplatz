// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT TheoreticalPlate {
    public:
        TheoreticalPlate();
        TheoreticalPlate( const TheoreticalPlate& );

        double ntp() const;
        void ntp( double );

        double baselineStartTime() const;
        void baselineStartTime( double );

        double baselineStartHeight() const;
        void baselineStartHeight( double );

        double baselineEndTime() const;
        void baselineEndTime( double );

        double baselineEndHeight() const;
        void baselineEndHeight( double );

        double peakTopTime() const;
        void peakTopTime( double );

        double peakTopHeight() const;
        void peakTopHeight( double );

    private:
        double ntp_;
        double ntpBaselineStartTime_;
        double ntpBaselineStartHeight_;
        double ntpBaselineEndTime_;
        double ntpBaselineEndHeight_;
        double ntpPeakTopTime_;
        double ntpPeakTopHeight_;

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            (void)version;
	    ar & BOOST_SERIALIZATION_NVP( ntp_ );
	    ar & BOOST_SERIALIZATION_NVP( ntpBaselineStartTime_ );
	    ar & BOOST_SERIALIZATION_NVP( ntpBaselineStartHeight_ );
	    ar & BOOST_SERIALIZATION_NVP( ntpBaselineEndTime_ );
	    ar & BOOST_SERIALIZATION_NVP( ntpBaselineEndHeight_ );
	    ar & BOOST_SERIALIZATION_NVP( ntpPeakTopTime_ );
	    ar & BOOST_SERIALIZATION_NVP( ntpPeakTopHeight_ );
        }
    };

}


