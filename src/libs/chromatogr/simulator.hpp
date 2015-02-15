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
#include <cstdint>
#include <vector>

namespace chromatogr {

    namespace simulator {

        class CHROMATOGRSHARED_EXPORT peak {
            double retention_time_;
            double theoretical_plate_;
            double height_;
            double sigma_;
            double distribution_height_;
        public:
            peak& operator = ( const peak& );
            peak( double retention_time, double theoretical_plate = 10000.0, double height = 1000.0 );
            double intensity( double t ) const;
        };

        class CHROMATOGRSHARED_EXPORT peaks {
#if _MSC_VER
# pragma warning(disable:4251)
#endif
            std::vector< peak > peaks_;
#if _MSC_VER
# pragma warning(default:4251)
#endif
        public:
            typedef std::vector< peak >::iterator iterator;
            typedef std::vector< peak >::const_iterator const_iterator;

            void operator << ( const peak& );
            size_t size() const;
            iterator begin() { return peaks_.begin(); }
            iterator end() { return peaks_.end(); }
            const_iterator begin() const { return peaks_.begin(); }
            const_iterator end() const { return peaks_.end(); }
            double intensity( double t ) const;
        };

    }
}


