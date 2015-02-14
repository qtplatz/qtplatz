/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "chromatogr_global.hpp"

namespace chromatogr {

    namespace simulator {

        class CHROMATOGRSHARED_EXPORT Peak {
            const double retention_time_;
            const double theoretical_plate_;
            const double tailing_factor_;
            const double scale_factor_;
        public:
            Peak( double retention_time, double theoretical_plate = 10000.0, double scale_factor_ = 1.0, double tailing_factor = 1.0 );
            double intensity_at_a_time( double t );
        };

        template<typename T> class CHROMATOGRSHARED_EXPORT Chromatogram {
            std::vector< T > chromatogram_;
            double sampInterval_;
        public:
            Chromatogram( const std::vector< Peak >& peaks, double sampInterval );
            Chromatogram( const Chromatogram& t ) : chromatogram_( t.chromatogram_ ), sampInterval_( t.sampInterval_ ) {
            }
        };

        class CHROMATOGRSHARED_EXPORT Chromatography {
        public:
            ~Chromatography();
            Chromatography();
            template<typename T> void operator()( const std::vector< Peak >& peaks, Chromatogram<T>& chromatogram, double sampInterval = 1.0, double runLength = 60.0 );
            double intensity( const std::vector< Peak >& peaks, double time ) const;
    };
    
}

#endif // CHROMATOGRAPHY_HPP
