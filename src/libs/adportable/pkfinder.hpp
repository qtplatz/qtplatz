/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#include <string>

namespace adportable {

    struct pkfinder {

        template< class ForwardIt, class Compare, class T > std::pair< ForwardIt, ForwardIt >
        operator()( ForwardIt first, ForwardIt last, const T& lower_value, const T& upper_value, Compare comp ) {

            auto xfirst = std::lower_bound( first, last, lower_value, comp );

            if ( xfirst != last && comp( *xfirst, upper_value ) ) {

                auto xsecond = std::lower_bound( xfirst, last, upper_value, comp );
                return std::make_pair( xfirst, xsecond );

            }

            return std::make_pair( last, last );
        }

    };

}

