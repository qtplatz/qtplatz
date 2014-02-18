/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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
#include <tuple>

#if defined _MSC_VER
# pragma warning(disable:4251)
#endif

namespace adcontrols {

    class MassSpectra;
    class Chromatogram;

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

        typedef ADCONTROLSSHARED_EXPORT struct peak_type {
            uint32_t idx_;
            double mass_;
            double height_;
            bool flag_;
            peak_type( uint32_t idx, double m, double h ) : idx_(idx), mass_(m), height_(h), flag_( false ) {}
        };

        class ADCONTROLSSHARED_EXPORT ClusterData { // : public std::enable_shared_from_this< ClusterData > {
            std::vector< peak_type > peaks_;
        public:
            ClusterData( const ClusterData& );
            ClusterData();
            ~ClusterData();

            inline void push_back( const peak_type& pk ) { peaks_.push_back( pk ); }
            inline std::vector<peak_type>::iterator begin() { return peaks_.begin(); };
            inline std::vector<peak_type>::iterator end() { return peaks_.end(); };
            inline peak_type& font() { return peaks_.front(); }
            inline peak_type& back() { return peaks_.back(); }
            inline size_t size() const { return peaks_.size(); }
            inline void insert( std::vector< peak_type >::iterator it, const peak_type& pk ) { peaks_.insert( it, pk ); }
        };

        class ADCONTROLSSHARED_EXPORT ClusterFinder {
            ClusterFinder( const ClusterFinder& t, std::vector< std::shared_ptr< ClusterData > >& );

        public:
            ClusterFinder( const ClusterMethod& m, std::function<bool (int curr, int total)> );
            bool operator()( const MassSpectra&, std::vector< std::shared_ptr< ClusterData > >&  );

            std::vector< ClusterData >& clusters();
            const std::vector< ClusterData >& clusters() const;

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

}

#endif // SPECTROGRAM_HPP
