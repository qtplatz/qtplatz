// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef CHROMATOGRAM_H
#define CHROMATOGRAM_H

#include "adcontrols_global.h"
#include <boost/any.hpp>
#include <string>

namespace boost {
  namespace serialization {
	class access;
  }
}

namespace adcontrols {

    namespace internal {
        class ChromatogramImpl;
    }

    class Description;
    class Descriptions;

    class ADCONTROLSSHARED_EXPORT Chromatogram {
    public:
        ~Chromatogram();
        Chromatogram();
        Chromatogram( const Chromatogram& );
        Chromatogram& operator = ( const Chromatogram& );

        struct Event {
            size_t index;  // index since injection, should subtract dataDelayPoint in order to access dataArray;
            unsigned long value;
            Event( size_t idx = 0, unsigned long v = 0 ) : index( idx ), value( v ) {}
        private:
            friend class boost::serialization::access;
            template<class Archiver> void serialize(Archiver& ar, const unsigned int version);
        };

        size_t size() const;
        void resize( size_t );

        bool isConstantSampledData() const;
        void isConstantSampledData( bool );
        double timeFromSampleIndex( size_t sampleIndex ) const;
        double timeFromDataIndex( size_t sampleIndex ) const;
        size_t toSampleIndex( double time, bool closest = false ) const;
        size_t toDataIndex( double time, bool closest = false ) const;

        const double * getDataArray() const;
        const double * getTimeArray() const;
        size_t eventsCount() const;
        const Event& getEvent( size_t idx ) const;

        void setDataArray( const double * );
        void setTimeArray( const double * );
        void addEvent( const Event& );

        double sampInterval() const; // seconds
        void sampInterval( double );
        size_t minTimePoints() const;  // equivalent to minTime count as number of points under sampInterval
        double minTime() const;  // min, a.k.a. start delay time
        double maxTime() const;  // min

        // if time delay caused by tubing, compensate by this value
        // semi-micro UV cell has about 2-3uL volume, assume 1.0m x 0.1mmID tubing were used
        // for connecting MS, 7.85uL of volume := 4.7seconds under 100uL/min flow rate.
        // When flowrate is 400uL still 1.2seconds delay, which is larger than peak width on 
        // peaks for k' < 4 if column plate number > 5000.
        double tubingDelayTime() const; // min
        void minTimePoints( size_t );
        void minTime( double );  // min
        void maxTime( double );  // min
        void tubingDelayTime( double ); // min

        void addDescription( const Description& );
        const Descriptions& getDescriptions() const;

        const std::wstring& axisLabelHorizontal() const;
        const std::wstring& axisLabelVertical() const;
        void axisLabelHorizontal( const std::wstring& );
        void axisLabelVertical( const std::wstring& );
    
    private:
        friend class boost::serialization::access;
        template<class Archiver> void serialize(Archiver& ar, const unsigned int version);

        internal::ChromatogramImpl * pImpl_;
    };
  
}

#endif // CHROMATOGRAM_H
