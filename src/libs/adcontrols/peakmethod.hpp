// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <boost/variant.hpp>
#include <adcontrols/timeutil.hpp>
#include <vector>

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
            , ePeakEvent_Off
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
        
        enum eNoiseFilterMethod {
            eNoFilter
            , eDFTLowPassFilter
        };

    } // chromatography

    template<typename T> class TimedEvent_archive;
    template<typename T> class PeakMethod_archive;

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

        chromatography::eNoiseFilterMethod noiseFilterMethod() const;
        void noiseFilterMethod( chromatography::eNoiseFilterMethod );
        double cutoffFreqHz() const;
        void cutoffFreqHz( double );

        class ADCONTROLSSHARED_EXPORT TimedEvent {
        public:
            ~TimedEvent();
            TimedEvent();
            TimedEvent( seconds_t t, adcontrols::chromatography::ePeakEvent );
            TimedEvent( const TimedEvent& );

			double time( bool asMinutes = true ) const;
			void setTime( double time, bool asMinutes = true );
            chromatography::ePeakEvent peakEvent() const;
            void setPeakEvent( chromatography::ePeakEvent );
			bool isBool() const;
			bool isDouble() const;
			double doubleValue() const;
			bool boolValue() const;
            void setValue( bool );
            void setValue( double );

            static bool isBool( adcontrols::chromatography::ePeakEvent );
            static bool isDouble( adcontrols::chromatography::ePeakEvent );
			
        private:
            double time_;
            adcontrols::chromatography::ePeakEvent event_;

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
            boost::variant< bool, double > value_;

#if defined _MSC_VER
#pragma warning(pop)
#endif
            friend class TimedEvent_archive < TimedEvent > ;
            friend class TimedEvent_archive < const TimedEvent > ;
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, const unsigned int version);
        };
        
        typedef std::vector< TimedEvent >::iterator iterator_type;
        typedef std::vector< TimedEvent >::const_iterator const_iterator_type;
        typedef std::vector< TimedEvent >::size_type size_type;
        size_type size() const;
        iterator_type begin();
        iterator_type end();
        const_iterator_type begin() const;
        const_iterator_type end() const;
        PeakMethod& operator << ( const TimedEvent& );
        iterator_type erase( iterator_type );
        iterator_type erase( iterator_type first, iterator_type last );
        void sort();

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
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
        std::vector< TimedEvent > timedEvents_;
#if defined _MSC_VER
#pragma warning(pop)
#endif

        chromatography::eNoiseFilterMethod noiseFilterMethod_;
        double cutoffFreqHz_; // Hz
        friend class PeakMethod_archive < PeakMethod > ;
        friend class PeakMethod_archive < const PeakMethod > ;        
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version );
	};
    
}

BOOST_CLASS_VERSION( adcontrols::PeakMethod::TimedEvent,  2 )
BOOST_CLASS_VERSION( adcontrols::PeakMethod,  3 )
