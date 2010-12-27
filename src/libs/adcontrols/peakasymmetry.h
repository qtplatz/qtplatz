// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
//#include <boost/serialization/string.hpp>
//#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT PeakAsymmetry {
    public:
        PeakAsymmetry();
        PeakAsymmetry( const PeakAsymmetry& );
        double asymmetry() const;
        double startTime() const;
        double endTime() const;

    private:
        double peakAsymmetry_;
        double peakAsymmetryStartTime_;
        double peakAsymmetryEndTime_;

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP( peakAsymmetry_ );
                ar & BOOST_SERIALIZATION_NVP( peakAsymmetryStartTime_ );
                ar & BOOST_SERIALIZATION_NVP( peakAsymmetryEndTime_ );
            }
        }

    };

}

