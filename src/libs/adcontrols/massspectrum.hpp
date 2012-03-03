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

#pragma once

#include "adcontrols_global.h"
#include <boost/any.hpp>
#include <string>

namespace boost {
    namespace serialization {
        class access;
    }
    namespace archive { 
        class binary_oarchive; 
        class binary_iarchive;
    }
}

class portable_binary_oarchive;
class portable_binary_iarchive;

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
				, CentroidIsotopeSimulation
   };
   
   namespace internal {
      class MassSpectrumImpl;
   }
   
    class Description;
    class Descriptions;
    class MSCalibration;
    class MSProperty;
    
    class ADCONTROLSSHARED_EXPORT MassSpectrum {
    public:
       ~MassSpectrum();
       MassSpectrum();
       MassSpectrum( const MassSpectrum& );
       MassSpectrum& operator = ( const MassSpectrum& );
	   void clone( const MassSpectrum&, bool deep = false );
       static const wchar_t * dataClass() { return L"MassSpectrum"; }
	 
       size_t size() const;
       void resize( size_t );
       const double * getMassArray() const;
       const double * getIntensityArray() const;
       const double * getTimeArray() const;
       size_t compute_profile_time_array( double *, size_t ) const;

       void setMass( size_t idx, double mass );
       void setIntensity( size_t idx, double intensity );
       void setTime( size_t idx, double time );
       void setAcquisitionMassRange( double, double );
       void setMassArray( const double *, bool setRange = false );
       void setIntensityArray( const double * );
       void setTimeArray( const double * );
       const unsigned char * getColorArray() const;
       void setColorArray( const unsigned char * );
       bool isCentroid() const;
       void setCentroid( CentroidAlgorithm );

       MS_POLARITY polarity() const;
       void setPolarity( MS_POLARITY );

       void setCalibration( const adcontrols::MSCalibration& );
       const MSCalibration& calibration() const;

       void setMSProperty( const adcontrols::MSProperty& );
       const MSProperty& getMSProperty() const;

       template<class T> void set( const T& t );
       template<class T> const T& get();
       std::pair<double, double> getAcquisitionMassRange() const;
	   double getMinIntensity() const;
	   double getMaxIntensity() const;
       double getMass( size_t idx ) const;
       double getIntensity( size_t idx ) const;
       double getTime( size_t idx ) const;
	 
       void addDescription( const Description& );
       const Descriptions& getDescriptions() const;

       std::wstring saveXml() const;
       void loadXml( const std::wstring& );

       static bool archive( std::ostream&, const MassSpectrum& );
       static bool restore( std::istream&, MassSpectrum& );
	 
    private:
       friend class boost::serialization::access;
       template<class Archiver> void serialize(Archiver& ar, const unsigned int version);

       internal::MassSpectrumImpl * pImpl_;
    };

    template<> void MassSpectrum::serialize( portable_binary_oarchive&, const unsigned int );
    template<> void MassSpectrum::serialize( portable_binary_iarchive&, const unsigned int );
    
    typedef boost::shared_ptr<MassSpectrum> MassSpectrumPtr;   
   
}


