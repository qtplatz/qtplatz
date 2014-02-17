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

        struct ADCONTROLSSHARED_EXPORT ClusterData {
            double mass_;
            double time_;
            double sdMass_; // distribution for masses (assume gaussian distribution)
            double sdTime_; // distribution for times (assume gaussian distribution)
            double height_; // maximum hight of centroid peaks on spectra
            double volume_; // sum of centroid peaks on spectra
            size_t idFirst_; // first spectrum index of this cluster
            size_t idLast_;  // last spectrum index of this cluster

            ClusterData( double m = 0, double t = 0, double dM = 0, double dT = 0, double h = 0, double v = 0, size_t id0 = 0, size_t id1 = 0)
                : mass_(m), time_(t), sdMass_(dM), sdTime_(dT), height_(h), volume_(v), idFirst_(id0), idLast_(id1) {
            }
        };

        class ADCONTROLSSHARED_EXPORT ClusterFinder {
            ClusterFinder( const ClusterFinder& t );

        public:
            ClusterFinder( const ClusterMethod& m, std::function<bool (int curr, int total)> );
            bool operator()( const MassSpectra& );

            std::vector< ClusterData >& clusters();
            const std::vector< ClusterData >& clusters() const;

        private:
            std::function< bool( int curr, long total ) > progress_;
            std::vector< ClusterData > clusters_;
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
