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
#include "metric/prefix.hpp"
#include <functional>
#include <string>
#include <memory>
#include <vector>

namespace boost {
    namespace archive { 
        class binary_oarchive; 
        class binary_iarchive;
    }
    namespace serialization {
        class access;
    }
    namespace uuids {
        struct uuid;
    }
}

class portable_binary_oarchive;
class portable_binary_iarchive;

namespace adcontrols {

    class MassSpectrum;

    typedef std::shared_ptr<MassSpectrum> MassSpectrumPtr;   

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
                             , CentroidNative // instrument manufacturer's native algorithm 
    };

    namespace internal {
        class MassSpectrumImpl;
    }
    
    class annotation;
    class description;
    class descriptions;
    class MSCalibration;
    class MSProperty;
    class annotations;
	class ScanLaw;

    
    class ADCONTROLSSHARED_EXPORT MassSpectrum {
    public:
        ~MassSpectrum();
        MassSpectrum();
        MassSpectrum( const MassSpectrum& );
        MassSpectrum& operator = ( const MassSpectrum& );
        MassSpectrum& operator += ( const MassSpectrum& );

        // algorithm
        const static size_t npos = size_t(-1);
        size_t lower_bound( double mass, bool isMass = true ) const;
        size_t find( double mass, double tolerance ) const;
        
        // methods
        void clone( const MassSpectrum&, bool deep = false );
        static const wchar_t * dataClass() { return L"MassSpectrum"; }
        
        size_t size() const;
        void resize( size_t );

        const double * getMassArray() const;
        const double * getIntensityArray() const;
        const double * getTimeArray() const;
        const unsigned char * getColorArray() const;

        //double compute_mass( double time ) const;
        //size_t compute_profile_time_array( double *, size_t, metric::prefix pfx = metric::base ) const;
        size_t operator << ( const std::pair< double, double >& ); // add (mass,intensity), return index

        void setMass( size_t idx, double mass );
        void setIntensity( size_t idx, double intensity );
        void setTime( size_t idx, double time );
        void setColor( size_t idx, unsigned char color );
        void setAcquisitionMassRange( double, double );
        void setMassArray( const double *, bool setRange = false );
        void setIntensityArray( const double * );
        void setTimeArray( const double * );
		void setColorArray( const unsigned char * );
        bool isCentroid() const;
        void setCentroid( CentroidAlgorithm );

        void setIntensityArray( std::vector< double >&& );
        void setMassArray( std::vector< double >&& );
        void setTimeArray( std::vector< double >&& );

        // ---
        void setDataReaderUuid( const boost::uuids::uuid& );
        const boost::uuids::uuid& dataReaderUuid() const;
        
        // ---
        MS_POLARITY polarity() const;
        void setPolarity( MS_POLARITY );
        int mode() const;

        typedef double( mass_assign_t )( double time, int mode );
        typedef std::function< mass_assign_t > mass_assignee_t;
        
        bool assign_masses( mass_assignee_t );
        //void setScanLaw( const adcontrols::ScanLaw&, bool assignMasses = false );
        void setCalibration( const adcontrols::MSCalibration&, bool assignMasses = false );
        const MSCalibration& calibration() const;
        
        void setMSProperty( const adcontrols::MSProperty& );
        const MSProperty& getMSProperty() const;
        MSProperty& getMSProperty();

        void normalizeIntensities( uint32_t nImaginalAverage );

        // [[deprecated("use MSProperty,acceleratorVoltage&tDelay")]]
        //     const ScanLaw* scanLaw() const;
        
        template<class T> void set( const T& t );
        template<class T> const T& get();
        std::pair<double, double> getAcquisitionMassRange() const;
        double getMinIntensity() const;
        double getMaxIntensity() const;
        double getMass( size_t idx ) const;
        double getIntensity( size_t idx ) const;
        double getTime( size_t idx ) const;

		size_t getIndexFromTime( double seconds, bool closest = false ) const;

		int getColor( size_t idx ) const;
    
        void addDescription( const description& );
        void addDescription( description&& );
        const descriptions& getDescriptions() const;

        void set_annotations( const annotations& );
        const annotations& get_annotations() const;
		annotations& get_annotations();
        void addAnnotation( annotation&& a, bool uniq = true );

        int32_t protocolId() const;
        int32_t nProtocols() const;
        void protocolId( int32_t );
        void nProtocols( int32_t );

        int64_t rowid() const;
        void setRowid( int64_t );

        std::wstring saveXml() const;
        void loadXml( const std::wstring& );
        
        // adfs::cpio (serializer)
        static bool archive( std::ostream&, const MassSpectrum& );
        static bool restore( std::istream&, MassSpectrum& );

        MassSpectrum& getSegment( size_t fcn /* o..n */ );

        const MassSpectrum& getSegment( size_t fcn /* 0..n */ ) const;

        MassSpectrum * findProtocol( int32_t proto );
        const MassSpectrum * findProtocol( int32_t proto ) const;

        void clearSegments();
        size_t numSegments() const;
        void uuid( const char * uuid );
        const char * uuid() const;
        bool trim( adcontrols::MassSpectrum&, const std::pair<double, double>& massrange ) const;

        typedef std::vector< std::shared_ptr< MassSpectrum > >::iterator iterator;
        typedef std::vector< std::shared_ptr< MassSpectrum > >::const_iterator const_iterator;

        MassSpectrum& operator << ( std::shared_ptr< MassSpectrum >&& );
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        iterator erase( const_iterator first, const_iterator last );
        
    private:
        friend class boost::serialization::access;
        template<class Archiver> void serialize(Archiver& ar, const unsigned int version);
        
        internal::MassSpectrumImpl * pImpl_;
    };

    struct ADCONTROLSSHARED_EXPORT segments_helper {
        static double max_intensity( const MassSpectrum& );
		static double min_intensity( const MassSpectrum& );
        static void set_color( MassSpectrum&, size_t fcn, size_t idx, int color );
        static int  get_color( const MassSpectrum&, size_t fcn, size_t idx );
        static std::pair<int, int> base_peak_index( const MassSpectrum&, double lMass, double uMass );
        static std::pair<double, double> acquisition_time_range( const MassSpectrum& );
        static double get_mass( const MassSpectrum&, const std::pair< int, int >& );
        static double get_intensity( const MassSpectrum&, const std::pair< int, int >& );
        static bool add( MassSpectrum&, const MassSpectrum& );
        static bool normalize( MassSpectrum&, uint32_t imaginalNumAverage = 10000 );
    };
    
    template<> ADCONTROLSSHARED_EXPORT void MassSpectrum::serialize( portable_binary_oarchive&, const unsigned int );
    template<> ADCONTROLSSHARED_EXPORT void MassSpectrum::serialize( portable_binary_iarchive&, const unsigned int );

    template<class T> class segment_iterator {
        size_t pos_;
        T& ms_;
    public:
        segment_iterator( T& ms, size_t pos ) : pos_( pos ), ms_( ms ) {}
        bool operator != ( const segment_iterator& rhs ) const { 
			return pos_ != rhs.pos_;
		}
        const segment_iterator& operator ++ () { ++pos_; return *this; }
        operator T* () const { return pos_ == 0 ? &ms_ : &ms_.getSegment( pos_ - 1 ); }
    };
    
	template<class T = MassSpectrum > class segment_wrapper {
		T& ms_;
    public:
		typedef T value_type;
        typedef segment_iterator<T> iterator;
        typedef const segment_iterator<T> const_iterator;
		typedef T& reference;
		typedef const T& const_reference;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
		segment_wrapper( T& ms ) : ms_( ms ) {}
        inline iterator begin()             { return segment_iterator<T>(ms_, 0); }
        inline const_iterator begin() const { return segment_iterator<T>(ms_, 0); }
        inline iterator end()               { return segment_iterator<T>(ms_, ms_.numSegments() + 1); }
        inline const_iterator end() const   { return segment_iterator<T>(ms_, ms_.numSegments() + 1); }
		inline reference operator [] ( size_t idx )             { return idx == 0 ? ms_ : ms_.getSegment( idx - 1 ); }
		inline const_reference operator [] ( size_t idx ) const { return idx == 0 ? ms_ : ms_.getSegment( idx - 1 ); }
		inline size_type size() const { return ms_.numSegments() + 1; }
		inline size_type max_size() const { return ms_.numSegments() + 1; }
    };

}
