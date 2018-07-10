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

#ifndef PROFILE_TIME_ARRAY_HPP
#define PROFILE_TIME_ARRAY_HPP

#include "adcontrols_global.h"
#include "metric/prefix.hpp"
#include <functional>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT profile_time_array  {
    public:
        profile_time_array( std::function< double(size_t) > f, size_t size ) : timef_( f ) {
        }

        double operator [] ( size_t idx ) const {
            return timef_( idx );
        }
    private:
        std::function<double(size_t)> timef_;
        //std::size_t size_;
    };

}

#endif // PROFILE_TIME_ARRAY_HPP
