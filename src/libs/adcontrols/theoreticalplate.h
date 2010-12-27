// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
        double NTP() const;
        double NTPBaselineStartTime() const;
        double NTPBaselineStartHeight() const;
        double NTPBaselineEndTime() const;
        double NTPBaselineEndHeight() const;
        double NTPPeakTopTime() const;
        double NTPPeakTopHeight() const;

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
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP( ntp_ );
                ar & BOOST_SERIALIZATION_NVP( ntpBaselineStartTime_ );
                ar & BOOST_SERIALIZATION_NVP( ntpBaselineStartHeight_ );
                ar & BOOST_SERIALIZATION_NVP( ntpBaselineEndTime_ );
                ar & BOOST_SERIALIZATION_NVP( ntpBaselineEndHeight_ );
                ar & BOOST_SERIALIZATION_NVP( ntpPeaktopTime_ );
                ar & BOOST_SERIALIZATION_NVP( ntpPeaktopHeight_ );
            }
        }
    };

}


