// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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
#include <adcontrols/timeutil.hpp>

namespace adcontrols {
    namespace chromatography {

        enum ePeakWidthMethod {
            ePeakWidth_Unknown
            , ePeakWidth_HalfHeight       // Full Width Half Maximum
            , ePeakWidth_5PercentHeight   // Full Width 5% Maximum
            , ePeakWidth_10PercentHeight  // Full Width 10% Maximum
            , ePeakWidth_1Sigma           // 2 Sigma full width
            , ePeakWidth_2Sigma           // 4 Sigma full width
            , ePeakWidth_3Sigma           // 6 Sigma full width
            , ePeakWidth_Tangent
            , ePeakWidth_AreaHeight
        };
        
        enum ePeakResolutionMethod {
            ePeakResolution_5PercentHeight
            , ePeakResolution_Tangent,
        };
        
        enum ePharmacopoeia {
            ePHARMACOPOEIA_NotSpcified
            , ePHARMACOPOEIA_EP
            , ePHARMACOPOEIA_JP
            , ePHARMACOPOEIA_USP
        };

        enum ePeakEvent {
            ePeakEvent_Nothing
            , PeakEvent_Lock
            , ePeakEvent_ForcedBase
            , ePeakEvent_ShiftBase
            , ePeakEvent_VtoV
            , ePeakEvent_Tailing
            , ePeakEvent_Leading
            , ePeakEvent_Shoulder
            , ePeakEvent_NegativePeak
            , ePeakEvent_NegativeLock
            , ePeakEvent_HorizontalBase
            , ePeakEvent_PostHorizontalBase
            , ePeakEvent_ForcedPeak
            , ePeakEvent_Slope
            , ePeakEvent_MinWidth
            , ePeakEvent_MinHeight
            , ePeakEvent_MinArea
            , ePeakEvent_Drift
            , ePeakEvent_Elimination
            , ePeakEvent_Manual
        };

    } // chromatography

    class ADCONTROLSSHARED_EXPORT PeakMethod {
    public:
        ~PeakMethod(void);
        PeakMethod(void);
		PeakMethod(const PeakMethod &);

		PeakMethod & operator = ( const PeakMethod & rhs );
		bool operator == ( const PeakMethod & rhs ) const;
		bool operator != ( const PeakMethod & rhs ) const;

        double minimumHeight() const;
        void minimumHeight( double );
        double minimumArea() const;
        void minimumArea( double );
        double minimumWidth() const;
        void minimumWidth( double );
        double doubleWidthTime() const;
        void doubleWidthTime( double );
        double slope() const;
        void slope( double );
        double drift() const;
        void drift( double );
        double t0() const;
        void t0( double );
        adcontrols::chromatography::ePharmacopoeia pharmacopoeia() const;
        void pharmacopoeia( adcontrols::chromatography::ePharmacopoeia );

        adcontrols::chromatography::ePeakWidthMethod peakWidthMethod() const;
        void peakWidthMethod( adcontrols::chromatography::ePeakWidthMethod );

        adcontrols::chromatography::ePeakWidthMethod theoreticalPlateMethod() const;
        void theoreticalPlateMethod( adcontrols::chromatography::ePeakWidthMethod );

        class ADCONTROLSSHARED_EXPORT TimedEvent {
        public:
            ~TimedEvent();
            TimedEvent( minutes_t t = 0
                        , adcontrols::chromatography::ePeakEvent e = adcontrols::chromatography::ePeakEvent_Nothing
                        , double v = 0 );
            TimedEvent( const TimedEvent& );
        private:
            minutes_t minutes_;
            adcontrols::chromatography::ePeakEvent event_;
            double value_;
            friend class boost::serialization::access;
            template<class Archive>
                void serialize(Archive& ar, const unsigned int version) {
                using namespace boost::serialization;
                (void)version;
                ar & BOOST_SERIALIZATION_NVP( minutes_ )
                    & BOOST_SERIALIZATION_NVP( event_ )
                    & BOOST_SERIALIZATION_NVP( value_ );
            }
        };

	private:
        double minimumHeight_;
        double minimumArea_;
        double minimumWidth_;
        double doubleWidthTime_;
        double slope_;
        double drift_;
        double t0_;
        adcontrols::chromatography::ePharmacopoeia pharmacopoeia_;
        adcontrols::chromatography::ePeakWidthMethod peakWidthMethod_;
        adcontrols::chromatography::ePeakWidthMethod theoreticalPlateMethod_;

       friend class boost::serialization::access;
       template<class Archive>
           void serialize(Archive& ar, const unsigned int /* version */) {
           using namespace boost::serialization;
           ar & BOOST_SERIALIZATION_NVP( minimumHeight_ )
               & BOOST_SERIALIZATION_NVP( minimumArea_ )
               & BOOST_SERIALIZATION_NVP( minimumWidth_ )
               & BOOST_SERIALIZATION_NVP( doubleWidthTime_ )
               & BOOST_SERIALIZATION_NVP( slope_ )
               & BOOST_SERIALIZATION_NVP( drift_ )
               & BOOST_SERIALIZATION_NVP( t0_ )
               & BOOST_SERIALIZATION_NVP( pharmacopoeia_ )
               & BOOST_SERIALIZATION_NVP( peakWidthMethod_ )
               & BOOST_SERIALIZATION_NVP( theoreticalPlateMethod_ );
       }
       
	};
    
}
