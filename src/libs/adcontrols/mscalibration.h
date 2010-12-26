// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"

#include <vector>
#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSCalibration {
    public:
        MSCalibration();
        MSCalibration( const MSCalibration& );
        MSCalibration( const std::vector<double>& );

        const std::string& date() const;
        void date( const std::string& );

        const std::wstring& calibId() const;
        void calibId( const std::wstring& );

        const std::vector< double >& coeffs() const;
        void coeffs( const std::vector<double>& );
        
    private:
        std::string calibDate_;
        std::wstring calibId_;
#pragma warning (disable:4251)
        std::vector< double > coeffs_;
#pragma warning (default:4251)

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP(calibDate_);
                ar & BOOST_SERIALIZATION_NVP(calibId_);
                ar & BOOST_SERIALIZATION_NVP(coeffs_);
            }
        }
    };

}

