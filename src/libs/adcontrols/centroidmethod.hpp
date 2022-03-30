// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

	class ADCONTROLSSHARED_EXPORT CentroidMethod {
	public:

        enum eNoiseFilterMethod {
            eNoFilter
            , eDFTLowPassFilter
        };

		enum ePeakWidthMethod {
			ePeakWidthTOF
			, ePeakWidthProportional
			, ePeakWidthConstant
		};

        enum eAreaMethod {
            eAreaDa         // I * Da -- deprecated
            , eAreaTime       // I * ns --> default
            , eWidthNormalized  // A / width
            , eAreaPoint      // assume data interval := 1
        };

		~CentroidMethod(void);
		CentroidMethod(void);
		CentroidMethod(const CentroidMethod &);
		CentroidMethod & operator = ( const CentroidMethod & rhs );

		bool operator == ( const CentroidMethod & rhs ) const;
		bool operator != ( const CentroidMethod & rhs ) const;

		[[deprecated("no longer used")]] double baselineWidth() const;
		[[deprecated("no longer used")]] double rsConstInDa() const;
		[[deprecated("no longer used")]] double rsPropoInPpm() const;
		double rsTofInDa() const;
		double rsTofAtMz() const;

        std::tuple< double, double > peak_width( ePeakWidthMethod = ePeakWidthTOF ) const;
        void set_peak_width( const std::tuple< double, double >&, ePeakWidthMethod = ePeakWidthTOF );
        void set_peak_width( double, ePeakWidthMethod );

        bool processOnTimeAxis() const;
        [[deprecated("use peak_process_on_time")]] double rsInSeconds() const;
        [[deprecated("use set_peak_process_on_time")]] void setRsInSeconds( double );
        [[deprecated("use set_peak_process_on_time")]] void setProcessOnTimeAxis( bool );

        std::pair< bool, double > peak_process_on_time() const;
        void set_peak_process_on_time( std::pair< bool, double >&& );

		// double attenuation() const; not in use
		double peakCentroidFraction() const;
		ePeakWidthMethod peakWidthMethod() const;

		bool centroidAreaIntensity() const;
		void baselineWidth(double);
		void rsConstInDa(double);
		void rsPropoInPpm(double);
		void rsTofInDa(double);
		void rsTofAtMz(double);
		void attenuation(double);
		void peakWidthMethod(ePeakWidthMethod);
		void centroidAreaIntensity(bool);
		void peakCentroidFraction(double);

        eNoiseFilterMethod noiseFilterMethod() const;
        void noiseFilterMethod( eNoiseFilterMethod );
        double cutoffFreqHz() const;
        void cutoffFreqHz( double );

        std::pair< eNoiseFilterMethod, double > noise_filter() const;
        void set_noise_filter( std::pair< eNoiseFilterMethod, double >&& );

        eAreaMethod areaMethod() const;
        void areaMethod( eAreaMethod );

	private:
		double baselineWidth_; // depreicated
        double rsConstInDa_;
        double rsPropoInPpm_;
        double rsTofInDa_;
		double rsTofAtMz_;
        double attenuation_; // not in use
		bool bCentroidAreaIntensity_;
		double peakCentroidFraction_;
        ePeakWidthMethod peakWidthMethod_;

        eNoiseFilterMethod noiseFilterMethod_; // since v2.2.3
        double cutoffFreqHz_; // Hz

        // CLASS VERSION 3
        eAreaMethod areaMethod_; // since v2.7.5

        // CLASS VERSION 4
        bool processOnTimeAxis_;
        double rsInSeconds_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            if ( version < 5 )
                ar & BOOST_SERIALIZATION_NVP(baselineWidth_); // deprecated
            ar & BOOST_SERIALIZATION_NVP(rsConstInDa_);
            ar & BOOST_SERIALIZATION_NVP(rsPropoInPpm_);
            ar & BOOST_SERIALIZATION_NVP(rsTofInDa_);
            ar & BOOST_SERIALIZATION_NVP(rsTofAtMz_);
            if ( version < 5 )
                ar & BOOST_SERIALIZATION_NVP(attenuation_); // deprecated

            ar & BOOST_SERIALIZATION_NVP(bCentroidAreaIntensity_);
            ar & BOOST_SERIALIZATION_NVP(peakCentroidFraction_);

            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP(noiseFilterMethod_)
                    & BOOST_SERIALIZATION_NVP(cutoffFreqHz_)
                    ;
            }
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( areaMethod_ );
            }
            if ( version >= 4 ) {
                ar & BOOST_SERIALIZATION_NVP( processOnTimeAxis_ );
                ar & BOOST_SERIALIZATION_NVP( rsInSeconds_ );
            }
            if ( version >= 5 ) {
                ar & BOOST_SERIALIZATION_NVP( peakWidthMethod_ ); // this was forgotten
            }
        }

	};

}

BOOST_CLASS_VERSION( adcontrols::CentroidMethod, 5 )
