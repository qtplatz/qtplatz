/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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
#include "mspeaks.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <string>
#include <memory>
#include <array>
#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSIonFocus {
    public:
        ~MSIonFocus();
        MSIonFocus();
        MSIonFocus( const MSIonFocus& );
        static const wchar_t * dataClass() { return L"adcontrols::MSIonFocus"; }

        enum eTolerance {
            ToleranceInDa
            , ToleranceInTime
            , ToleranceInPeakWidth
            , nToleranceMethod
        };

        // centroid method
        double acceleratorVoltage() const;
        double timeOffset() const;
        bool   hasCalibration() const;
        int32_t mode() const;
        double fLength() const;
        eTolerance toleranceMethod() const;
        double tolerance( eTolerance ) const;

        const MSPeaks& expected() const;
        const MSPeaks& assigned() const;
        MSPeaks& expected();
        MSPeaks& assigned();

        void acceleratorVoltage( double );
        void timeOffset( double );
        void hasCalibration( bool );
        void mode( int32_t );
        void fLength( double );
        void toleranceMethod( eTolerance );
        void tolerance( eTolerance, double );

    private:
        double acceleratorVoltage_;
        double timeOffset_; // t0
        bool   hasCalibration_;
        int32_t mode_;
        double fLength_;
        eTolerance toleranceMethod_;
        std::array< double, nToleranceMethod > tolerances_;
        MSPeaks expected_;
        MSPeaks assigned_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive& ar, const unsigned int version) {
            (void)(version);
            ar & acceleratorVoltage_
                & timeOffset_
                & hasCalibration_
                & mode_
                & fLength_
                & toleranceMethod_
                & tolerances_
                & expected_
                & assigned_
                ;
        }
    };

}


