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
#include <boost/any.hpp>
#include <string>
#include <memory>

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

    namespace internal {
        class ChromatogramImpl;
    }

    class Description;
    class Descriptions;
    class Peaks;
    class Baselines;

    class ADCONTROLSSHARED_EXPORT Chromatogram {
    public:
        ~Chromatogram();
        Chromatogram();
        Chromatogram( const Chromatogram& );
        Chromatogram& operator = ( const Chromatogram& );

        struct seconds_t { seconds_t( double t ) : seconds(t) {} double seconds; operator double () const { return seconds; } };
        struct minutes_t { minutes_t( double t ) : minutes(t) {} double minutes; operator double () const { return minutes; } };

        static minutes_t toMinutes( const seconds_t& );
        static seconds_t toSeconds( const minutes_t& );
        static std::pair<double, double> toMinutes( const std::pair< seconds_t, seconds_t >& );

        static bool archive( std::ostream&, const Chromatogram& );
        static bool restore( std::istream&, Chromatogram& );
		static const wchar_t * dataClass() { return L"Chromatogram"; }

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

        double getMinIntensity() const;
        double getMaxIntensity() const;
        size_t min_element( size_t beg = 0, size_t end = (-1) ) const;
        size_t max_element( size_t beg = 0, size_t end = (-1) ) const;

        const double * getIntensityArray() const;
        const double * getTimeArray() const;
        size_t eventsCount() const;
        const Event& getEvent( size_t idx ) const;

        void setFcn( int );
        int fcn() const;

        void setIntensity( size_t idx, double );
        void setTime( size_t idx, double );
        void setIntensityArray( const double * );
        void setTimeArray( const double * );
        void addEvent( const Event& );

        seconds_t sampInterval() const; // seconds
        void sampInterval( const seconds_t&  );

        size_t minimumTimePoints() const;  // equivalent to minTime count as number of points under sampInterval
        seconds_t minimumTime() const;  // a.k.a. start delay time
		seconds_t maximumTime() const;  // 
        std::pair<seconds_t, seconds_t> timeRange() const;

        // if time delay caused by tubing, compensate by this value
        // semi-micro UV cell has about 2-3uL volume, assume 1.0m x 0.1mmID tubing were used
        // for connecting MS, 7.85uL of volume := 4.7seconds under 100uL/min flow rate.
        // When flowrate is 400uL still 1.2seconds delay, which is larger than peak width on 
        // peaks for k' < 4 if column plate number > 5000.
        double tubingDelayTime() const; // min
        void minimumTimePoints( size_t );
        void minimumTime( const seconds_t& );
        void maximumTime( const seconds_t& );
        void tubingDelayTime( const seconds_t& ); // min

        void addDescription( const Description& );
        const Descriptions& getDescriptions() const;

        const std::wstring& axisLabelHorizontal() const;
        const std::wstring& axisLabelVertical() const;
        void axisLabelHorizontal( const std::wstring& );
        void axisLabelVertical( const std::wstring& );

        Peaks& peaks();
        const Peaks& peaks() const;

        Baselines& baselines();
        const Baselines& baselines() const;
    
    private:
        friend class boost::serialization::access;
        template<class Archiver> void serialize(Archiver& ar, const unsigned int version);

        internal::ChromatogramImpl * pImpl_;
    };

    template<> void Chromatogram::serialize( portable_binary_oarchive&, const unsigned int );
    template<> void Chromatogram::serialize( portable_binary_iarchive&, const unsigned int );

    typedef std::shared_ptr<Chromatogram> ChromatogramPtr;   

}

