// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
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
#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

namespace adcontrols {

#if defined _MSC_VER
    struct CountingPeak;
    template class ADCONTROLSSHARED_EXPORT std::vector< adcontrols::CountingPeak >;
#endif    

    struct ADCONTROLSSHARED_EXPORT CountingPeak {

        std::tuple<
            std::pair< double, double >  // apex<time,intensity>
            , std::pair< int, double >   // front <x offset samples from apex, intensity>
            , std::pair< int, double >   // back  <x offset samples from apex, intensity>
            >  d_;

        enum pk_item { pk_apex, pk_front, pk_back, /**/ pk_area, pk_height };

        CountingPeak();
        CountingPeak( const CountingPeak& t );

        std::pair< double, double > apex() const { return std::get< pk_apex >( d_ ); }
        std::pair< double, double >& apex()      { return std::get< pk_apex >( d_ ); }
        
        std::pair< int, double > front() const   { return std::get< pk_front >( d_ ); }
        std::pair< int, double >& front()        { return std::get< pk_front >( d_ ); }
        
        std::pair< int, double > back() const    { return std::get< pk_back >( d_ ); }
        std::pair< int, double >& back()         { return std::get< pk_back >( d_ ); }

        double area() const;
        double height() const;
        double width() const;
    };

    class ADCONTROLSSHARED_EXPORT CountingData {
    public:
        CountingData();
        CountingData( const CountingData& );
        CountingData& operator = ( const CountingData& );

        uint32_t triggerNumber() const;
        uint32_t protocolIndex() const;
        uint64_t timeSinceEpoch() const;
        double elapsedTime() const;
        uint32_t events() const;
        double threshold() const;
        uint32_t algo() const; // 0:Absolute, 1:Average, 2:Differential

        void setTriggerNumber( uint32_t );
        void setProtocolIndex( uint32_t );
        void setTimeSinceEpoch( uint64_t );
        void setElapsedTime( double );
        void setEvents( uint32_t );
        void setThreshold( double );
        void setAlgo( uint32_t );

        std::vector< CountingPeak >& peaks() { return peaks_; }
        const std::vector< CountingPeak >& peaks() const { return peaks_; }

    private:
        uint32_t triggerNumber_;
        uint32_t protocolIndex_;
        uint64_t timeSinceEpoch_;
        double elapsedTime_;
        uint32_t events_;
        double threshold_;
        uint32_t algo_;
        std::vector< CountingPeak > peaks_;
    };

    namespace counting {

        struct ADCONTROLSSHARED_EXPORT Stat {
            double mean;
            double stddev;
            double minValue;
            double maxValue;
            size_t count;
            Stat( double _m = 0, double _sd = 0, double _min = 0, double _max = 0, size_t _count = 0 )
                : mean( _m ), stddev( _sd ), minValue( _min ), maxValue( _max ), count( _count )
            {}
        };

        template< CountingPeak::pk_item item >
        struct ADCONTROLSSHARED_EXPORT statistics {

            template< typename iterator > Stat operator()( iterator begin, iterator end ) {

                size_t count = std::distance( begin, end );

                auto range = std::minmax_element(
                    begin, end, []( const CountingPeak& a, const CountingPeak& b ){
                        return std::get< item >( a.d_ ).second < std::get< item >( b.d_ ).second;  });

                double mean = std::accumulate( begin, end, 0.0
                                               , []( double b, const CountingPeak& a ){
                                                   return std::get< item >( a.d_ ).second + b; }) / count;

                double sd2 = std::accumulate( begin, end, 0.0, [&]( double b, const CountingPeak& a ){
                        double v = ( std::get< item >( a.d_ ).second - mean );
                        return b + ( v * v ); })  / count;

                return Stat( mean, std::sqrt( sd2 )
                             , std::get< item >( range.first->d_ ).second
                             , std::get< item >( range.second->d_ ).second, count );
            }
        };

        template<> struct statistics< CountingPeak::pk_area > {
            template< typename iterator > Stat operator()( iterator begin, iterator end ) {
                size_t count = std::distance( begin, end );

                auto range = std::minmax_element(
                    begin, end, []( const CountingPeak& a, const CountingPeak& b ){ return a.area() < b.area();  });

                double mean = std::accumulate( begin, end, 0.0
                                               , []( double b, const CountingPeak& a ){ return a.area() + b; }) / count;

                double sd2 = std::accumulate( begin, end, 0.0, [&]( double b, const CountingPeak& a ){
                        return b + ( a.area() - mean ) * ( a.area() - mean ); }) / count;
                
                return Stat( mean, std::sqrt( sd2 ), range.first->area(), range.second->area(), count );
            }
        };

        template<> struct statistics< CountingPeak::pk_height > {
            template< typename iterator > Stat operator()( iterator begin, iterator end ) {
                size_t count = std::distance( begin, end );

                auto range = std::minmax_element(
                    begin, end, []( const CountingPeak& a, const CountingPeak& b ){ return a.height() < b.height();  });

                double mean = std::accumulate( begin, end, 0.0
                                               , []( double b, const CountingPeak& a ){ return a.height() + b; }) / count;
                
                double sd2 = std::accumulate( begin, end, 0.0, [&]( double b, const CountingPeak& a ){
                        return b + ( a.height() - mean ) * ( a.height() - mean ); }) / count;
                
                return Stat( mean, std::sqrt( sd2 ), range.first->height(), range.second->height(), count );
            }
        };

    }
    
}

