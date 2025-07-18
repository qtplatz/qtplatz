// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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
#include "constants.hpp"
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace boost {
    namespace serialization {
        class access;
    }
    namespace archive {
        class binary_oarchive;
        class binary_iarchive;
    }
    namespace uuids {
        struct uuid;
    }
}

class portable_binary_oarchive;
class portable_binary_iarchive;

namespace adcontrols {
    // namespace internal {
    //     class ChromatogramImpl;
    // }

    class description;
    class descriptions;
    class Peak;
    class Peaks;
    class Baseline;
    class Baselines;
    class PeakResult;
    class Chromatogram_iterator;
    class moltable;

    class ADCONTROLSSHARED_EXPORT Chromatogram;  // workaround for emacs auto indent bug

    class Chromatogram {
    public:
        static const constexpr size_t npos = std::size_t(-1);
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
        static const boost::uuids::uuid __clsid__();

        struct Event {
            size_t index;  // index since injection, should subtract dataDelayPoint in order to access dataArray;
            unsigned long value;
            Event( size_t idx = 0, unsigned long v = 0 ) : index( idx ), value( v ) {}
        private:
            friend class boost::serialization::access;
            template<class Archiver> void serialize(Archiver& ar, const unsigned int version);
        };

        template< typename T > static std::basic_string< T > make_folder_name( const adcontrols::descriptions& );
        std::string make_title() const;

        typedef Chromatogram_iterator iterator;
        typedef const Chromatogram_iterator const_iterator;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        size_t size() const;
        void resize( size_t );

        bool isConstantSampledData() const;
        void setIsConstantSampledData( bool );

        double timeFromSampleIndex( size_t sampleIndex ) const;
        double timeFromDataIndex( size_t sampleIndex ) const;
        size_t toSampleIndex( double time, bool closest = false ) const;
        size_t toDataIndex( double time, bool closest = false ) const;
        std::pair< size_t,size_t> toIndexRange( const std::pair< double, double >& ) const;

        double getMinIntensity() const;
        double getMaxIntensity() const;
        size_t min_element( size_t beg = 0, size_t end = (-1) ) const;
        size_t max_element( size_t beg = 0, size_t end = (-1) ) const;

        const std::vector< double >& timeArray() const;
        std::vector< double >& timeArray();

        const double * getIntensityArray() const;
        const double * getTimeArray() const;
        size_t eventsCount() const;
        const Event& getEvent( size_t idx ) const;

        const std::vector< double >& tofArray() const;
        const std::vector< double >& massArray() const;
        double tof( size_t idx ) const;
        double mass( size_t idx ) const;

        void setProtocol( int );
        int protocol() const;

        double time( size_t idx ) const;
        double intensity( size_t idx ) const;
        std::pair< double, double > datum( size_t idx ) const;
        void setIntensity( size_t idx, double );
        void setTime( size_t idx, double );
        void setDatum( size_t idx, std::pair< double, double >&& );
        void setIntensityArray( const double *, size_t sz );
        void setTimeArray( const double *, size_t sz );
        void addEvent( const Event& );

        // append (time,intensity) to the end of chromatogram
        void operator << ( const std::pair<double, double>& );
        void operator << ( std::pair<double, double>&& );
        void operator << ( std::tuple<double, double, double, double>&& ); // time,inens,tof,mass

        seconds_t sampInterval() const; // seconds
        void setSampInterval( const seconds_t&  );

        size_t minimumTimePoints() const;  // equivalent to minTime count as number of points under sampInterval
        seconds_t minimumTime() const;  // a.k.a. start delay time
		seconds_t maximumTime() const;  //
        std::pair<seconds_t, seconds_t> timeRange() const;

        void minimumTimePoints( size_t );
        void setMinimumTime( const seconds_t& );
        void setMaximumTime( const seconds_t& );
        // void tubingDelayTime( const seconds_t& ); // min

        void addDescription( const description& );
        void addDescription( description&& );
        void addDescription( std::pair< std::string, std::string >&& );
        const descriptions& getDescriptions() const;
        // const adcontrols::descriptions& descriptions() const; <-- not be able to compile on gcc

        boost::optional< std::string > axisLabel( plot::axis ) const;
        void setAxisLabel( plot::axis, const std::string& );

        std::pair< plot::unit, size_t > axisUnit() const;
        void setAxisUnit( plot::unit, size_t den = 0 );

        void setDataReaderUuid( const boost::uuids::uuid& );
        const boost::uuids::uuid& dataReaderUuid() const;

        void setDataGuid( const boost::uuids::uuid& );
        const boost::uuids::uuid& dataGuid() const;

        void setGeneratorProperty( const std::string& );
        void setGeneratorProperty( std::string&& );
        boost::optional< std::string > generatorProperty() const;

        void set_display_name( const std::string& );
        boost::optional< std::string > display_name() const;

        bool add_manual_peak( PeakResult&, double t0, double t1, bool horizontalBaseline = true, double baseLevel = 0 ) const;

        // find peak from given range (t0, t1), and find peak left and right
        std::pair< std::shared_ptr< Peak >, std::shared_ptr< Baseline > >
        find_single_peak( double t0, double t1, bool horizontalBaseline = true, double baseLevel = 0 ) const;

        const Peaks& peaks() const;
        const Baselines& baselines() const;

        void setBaselines( const Baselines& );
        void setPeaks( const Peaks& );
        void setSinglePeak( std::pair< std::shared_ptr< Peak >, std::shared_ptr< Baseline > >&& );

        void setIsCounting( bool );
        bool isCounting() const;

        void set_time_of_injection( std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >&& );
        void set_time_of_injection( const std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds >& );
        std::chrono::time_point< std::chrono::system_clock, std::chrono::nanoseconds> time_of_injection() const;
        void set_time_of_injection_iso8601( const std::string& );
        std::string time_of_injection_iso8601() const;

        // adjusting actual injection delay time since SFE start, when SFE and SFC are both recorded on the dataset
        bool set_sfe_injection_delay( bool, double s ); // seconds (intanally, ns with int64)
        std::optional< double > sfe_injection_delay() const;

        template< typename T >
        void setIntensityArray( const std::vector< T >& data, double factor = 1.0 ) {
            resize( data.size() );
            std::transform( data.begin(), data.end(), intensVector().begin(), [&](const auto& a){ return double(a * factor); });
        }

    private:
        friend class boost::serialization::access;
        template<class Archiver> void serialize(Archiver& ar, const unsigned int version);
        class impl;
        impl * impl_;
        std::vector< double >& intensVector();
    };

    template<> void Chromatogram::serialize( portable_binary_oarchive&, const unsigned int );
    template<> void Chromatogram::serialize( portable_binary_iarchive&, const unsigned int );

    typedef std::shared_ptr<Chromatogram> ChromatogramPtr;

    class ADCONTROLSSHARED_EXPORT Chromatogram_iterator {
        const Chromatogram * chromatogram_;
        size_t idx_;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Chromatogram_iterator;
#if __cplusplus >= 201703L
        using difference_type = std::ptrdiff_t;
#else
        using difference_type = int;
#endif
        using pointer = value_type*;
        using reference = value_type&;

        Chromatogram_iterator();
        Chromatogram_iterator( const Chromatogram *, size_t idx );
        Chromatogram_iterator( const Chromatogram_iterator& );
        Chromatogram_iterator& operator = ( const Chromatogram_iterator& );
        const Chromatogram_iterator& operator ++ ();
        const Chromatogram_iterator operator ++ ( int );
        inline bool operator == ( const Chromatogram_iterator& rhs ) const { return idx_ == rhs.idx_; }
        inline bool operator != ( const Chromatogram_iterator& rhs ) const { return idx_ != rhs.idx_; }
        inline double time() const { return chromatogram_->time( idx_ ); }
        inline double intensity() const { return chromatogram_->intensity( idx_ ); };
    };

    template<> ADCONTROLSSHARED_EXPORT std::basic_string< char > Chromatogram::make_folder_name( const adcontrols::descriptions& );
    template<> ADCONTROLSSHARED_EXPORT std::basic_string< wchar_t > Chromatogram::make_folder_name( const adcontrols::descriptions& );

}
