/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef SPECTROGRAM_HPP
#define SPECTROGRAM_HPP

#include "adcontrols_global.h"
#include <vector>
#include <functional>
#include <cstdint>
#include <memory>

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#if defined _MSC_VER
# pragma warning(disable:4251)
#endif

namespace adcontrols {

    class MassSpectra;
    class Chromatogram;
    class SpectrogramClusters;
    typedef std::shared_ptr< SpectrogramClusters > SpectrogramClustersPtr;

    class ADCONTROLSSHARED_EXPORT Spectrogram  {
    public:
        Spectrogram();

        struct ADCONTROLSSHARED_EXPORT ClusterMethod {
            double massWindow_; // daltons to be marged
            double timeWindow_; // seconds
            double lMassLimit_;
            double hMassLimit_;
            ClusterMethod( double mw = 0.001, double tw = 60.0, double lMass = 0, double hMass = 0 )
                : massWindow_(mw), timeWindow_(tw), lMassLimit_( lMass ), hMassLimit_( hMass ) {
            } 
        };

        struct ADCONTROLSSHARED_EXPORT peak_type {
            uint32_t idx_;
            double mass_;
            double height_;
            bool flag_;
            peak_type( uint32_t idx = 0, double m = 0, double h = 0 ) : idx_(idx), mass_(m), height_(h), flag_( false ) {}

        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, const unsigned int ) {
                ar & BOOST_SERIALIZATION_NVP( idx_ ) & BOOST_SERIALIZATION_NVP( mass_ ) & BOOST_SERIALIZATION_NVP( height_ );
            }
        };

        class ADCONTROLSSHARED_EXPORT ClusterData { // : public std::enable_shared_from_this< ClusterData > {
        public:
            ClusterData( const ClusterData& );
            ClusterData();
            ~ClusterData();

            inline void push_back( const peak_type& pk ) { peaks_.push_back( pk ); }
            inline std::vector<peak_type>::iterator begin() { return peaks_.begin(); };
            inline std::vector<peak_type>::iterator end() { return peaks_.end(); };
            inline peak_type& front() { return peaks_.front(); }
            inline peak_type& back() { return peaks_.back(); }
            inline size_t size() const { return peaks_.size(); }
            inline void insert( std::vector< peak_type >::iterator it, const peak_type& pk ) { peaks_.insert( it, pk ); }
            inline const std::pair< double, double >& mass_interval() const { return mass_interval_; }
            inline void mass_interval( double min, double max ) { mass_interval_ = std::make_pair( min, max ); }
            inline const std::pair< double, double >& time_interval() const { return time_interval_; }
            inline void time_interval( double min, double max ) { time_interval_ = std::make_pair( min, max ); }
            uint32_t center_index() const;
        private:
            std::vector< peak_type > peaks_;
            std::pair< double, double > mass_interval_;
            std::pair< double, double > time_interval_;

            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, const unsigned int ) {
                ar & BOOST_SERIALIZATION_NVP( peaks_ )
                    & BOOST_SERIALIZATION_NVP( mass_interval_ )
                    & BOOST_SERIALIZATION_NVP( time_interval_ );
            };
        };

        class ADCONTROLSSHARED_EXPORT ClusterFinder {
            ClusterFinder( const ClusterFinder& t, std::vector< std::shared_ptr< ClusterData > >& );

        public:
            ClusterFinder( const ClusterMethod& m, std::function<bool (int curr, int total)> );
            bool operator()( const MassSpectra&, SpectrogramClusters& );

        private:
            std::function< bool( int curr, long total ) > progress_;
            std::vector< std::shared_ptr< ClusterData > > work_;
            ClusterMethod method_;
        };
        
        class ChromatogramExtractor {
        public:
            ChromatogramExtractor( const MassSpectra& spectra );
            void operator()( Chromatogram& c, double lMass, double hMass );
        private:
            const MassSpectra& spectra_;
            std::vector< double > seconds_;
        };
    };

    //////////////////

    class ADCONTROLSSHARED_EXPORT SpectrogramClusters : public std::enable_shared_from_this< SpectrogramClusters > {
#if defined _MSC_VER && _MSC_VER <= 1700
        SpectrogramClusters( const SpectrogramClusters& );
#else
        SpectrogramClusters( const SpectrogramClusters& ) = delete;
#endif
    public:
        static const wchar_t * dataClass() { return L"adcontrols::SpectrogramClusters"; }

        SpectrogramClusters();
        void clone( const SpectrogramClusters& );

        void operator << ( const Spectrogram::ClusterData& );
        typedef std::vector< Spectrogram::ClusterData >::iterator iterator;
        typedef std::vector< Spectrogram::ClusterData >::const_iterator const_iterator;
        size_t size() const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        static bool archive( std::ostream&, const SpectrogramClusters& );
        static bool restore( std::istream&, SpectrogramClusters& );
        
    private:
        std::vector< Spectrogram::ClusterData > data_;

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( data_ );
        };
    };
}

#endif // SPECTROGRAM_HPP
