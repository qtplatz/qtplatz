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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include "mscalibration.h"

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSProperty {
    public:
        MSProperty();
        MSProperty( const MSProperty& );

        double accelerateVoltage() const;
        void setAccelerateVoltage( double );

        // number of average for waveform
        size_t numAverage() const;
        void setNumAverage( size_t );

        double time( size_t pos ); // return flight time for data[pos] in seconds

        size_t instSamplingInterval() const; // ps
        void setInstSamplingInterval( size_t ); // ps

        size_t instSamplingStartDelay() const;  // number of data points before record waveform in array
        void setInstSamplingStartDelay( size_t );

        unsigned long timeSinceInjection() const;
        void setTimeSinceInjection( unsigned long );

        // acquisition mass range, usually it is from user parameter based on theoretical calibration
        std::pair<double, double> instMassRange() const;
        void setInstMassRange( const std::pair<double, double>& );

    private:
        unsigned long time_since_injection_; // msec
        double instAccelVoltage_;
        size_t instNumAvrg_;
        size_t instSamplingStartDelay_;
        size_t instSamplingInterval_; // ps

        // bool bLockmassApplied_;
        // MSCalibration instCalib_;
        // double massFactor_;
        // double interpolateFromTime_;	// interpolation range for calibration
        // double interpolateToTime_;		// interpolation range for calibration
        // double instAcqCalibMax_; // 1,000 | 2,000 | 4,000 | 10,000
        // double instTimeOffset_; // seconds
        // double instSampIntval_; // seconds  0.5e-9, 1.0e-9, 2.0e-9 ...
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP(time_since_injection_);
                ar & BOOST_SERIALIZATION_NVP(instAccelVoltage_);
                ar & BOOST_SERIALIZATION_NVP(instNumAvrg_);
                ar & BOOST_SERIALIZATION_NVP(instSamplingStartDelay_);
                ar & BOOST_SERIALIZATION_NVP(instSamplingInterval_);
            }
        }

    };

}

BOOST_CLASS_VERSION(adcontrols::MSProperty, 1)


