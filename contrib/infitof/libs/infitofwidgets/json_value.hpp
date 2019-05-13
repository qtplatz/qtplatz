/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include <boost/property_tree/ptree.hpp>

namespace infitofwidgets {

    struct json_value {
        template< typename T > static
        std::pair< boost::optional< T >, boost::optional< T > > get_optional( const boost::property_tree::ptree& pt ) {
            return { pt.get_optional< T >( "set" ), pt.get_optional< T >( "act" ) };
        }

        template< typename T > static
        std::pair< T, T > get( const boost::property_tree::ptree& pt ) {
            std::pair< T, T > t = { 0, 0 };
            if ( auto v  = pt.get_optional< T >( "set" ) )
                t.first  = v.get();
            if ( auto v  = pt.get_optional< T >( "act" ) )
                t.second = v.get();
            return t;
        }
        
    };
}

