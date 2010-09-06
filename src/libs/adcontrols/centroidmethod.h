// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

	class ADCONTROLSSHARED_EXPORT CentroidMethod {
	public:

		enum ePeakWidthMethod {
			ePeakWidthTOF,
			ePeakWidthProportional,
			ePeakWidthConstant
		};

		~CentroidMethod(void);
		CentroidMethod(void);
		CentroidMethod(const CentroidMethod &);
		CentroidMethod & operator = ( const CentroidMethod & rhs );

		bool operator == ( const CentroidMethod & rhs ) const;
		bool operator != ( const CentroidMethod & rhs ) const;

		double baselineWidth() const;
		double rsConstInDa() const;
		double rsPropoInPpm() const;
		double rsTofInDa() const;
		double rsTofAtMz() const;
		double attenuation() const;
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

	private:
		double baselineWidth_;
        double rsConstInDa_;
        double rsPropoInPpm_;
        double rsTofInDa_;
		double rsTofAtMz_;
        double attenuation_;
		bool bCentroidAreaIntensity_;
		double peakCentroidFraction_;
        ePeakWidthMethod peakWidthMethod_;

       friend class boost::serialization::access;
       template<class Archive>
       void serialize(Archive& ar, const unsigned int version) {
           using namespace boost::serialization;
           if ( version >= 0 ) {
               ar & BOOST_SERIALIZATION_NVP(baselineWidth_);
               ar & BOOST_SERIALIZATION_NVP(rsConstInDa_);
               ar & BOOST_SERIALIZATION_NVP(rsPropoInPpm_);
               ar & BOOST_SERIALIZATION_NVP(rsTofInDa_);
			   ar & BOOST_SERIALIZATION_NVP(rsTofAtMz_);
			   ar & BOOST_SERIALIZATION_NVP(attenuation_);
			   ar & BOOST_SERIALIZATION_NVP(bCentroidAreaIntensity_);
			   ar & BOOST_SERIALIZATION_NVP(peakCentroidFraction_);
           }
       }

	};

}
