// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef MASSSPECTRUM_H
#define MASSSPECTRUM_H

#include "adcontrols_global.h"
#include <boost/any.hpp>
#include <string>

namespace adcontrols {

   enum MS_POLARITY { PolarityIndeterminate
		      , PolarityPositive = (1)
		      , PolarityNegative
		      , PolarityMixed
   };
   
   enum CentroidAlgorithm { CentroidNone
			    , CentroidPeakMass
			    , CentroidPeakAreaWaitedMass
			    , CentroidPeakMoment
   };
   
   namespace internal {
      class MassSpectrumImpl;
   }
   
   class Description;
   class Descriptions;

   class ADCONTROLSSHARED_EXPORT MassSpectrum {
      public:
	 ~MassSpectrum();
	 MassSpectrum();
	 MassSpectrum( const MassSpectrum& );
	 MassSpectrum& operator = ( const MassSpectrum& );
	 
	 size_t size() const;
	 void resize( size_t );
	 
	 const double * getMassArray() const;
	 const double * getIntensityArray() const;
	 const double * getTimeArray();
	 void setMassArray( const double * );
	 void setIntensityArray( const double * );
	 void setTimeArray( const double * );
	 const unsigned char * getColorArray() const;
	 void setColorArray( const unsigned char * );
	 
	 template<class T> void set( const T& t );
	 template<class T> const T& get();
	 
	 void addDescription( const Description& );
	 const Descriptions& getDescriptions() const;

	 std::wstring saveXml() const;
         void loadXml( const std::wstring& );
	 
      private:
	 internal::MassSpectrumImpl * pImpl_;
   };
   
}

#endif // MASSSPECTRUM_H
