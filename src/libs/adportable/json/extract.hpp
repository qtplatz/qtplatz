// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "../adportable_global.h"
#include <boost/json/value_to.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <type_traits>
#include <iostream>

namespace boost { namespace uuids { class uuid; } };

namespace adportable {
    namespace json {

        template<bool = true >
        struct workaround {
            template< typename T > void assign( T& t, boost::json::string_view value ) {
                t = boost::lexical_cast< T >( value );
                std::cerr << __FILE__ << ":" << __LINE__ << "\t## workaround assign type to " << std::string( typeid(t).name() )
                          << std::endl;
            }
        };

        template<>
        struct workaround< false > { template< typename T > void assign( T& t, boost::json::string_view value ) {} };

        ////////////////

        template<class T>
        void extract( const boost::json::object& obj, T& t, boost::json::string_view key )  {
            try {
                t = boost::json::value_to<T>( obj.at( key ) );
            } catch ( std::exception& ex ) {
                if ( obj.at( key ).is_string() && std::is_arithmetic< T >::value ) {
                    workaround< std::is_arithmetic< T >::value >().assign( t, obj.at( key ).as_string() );
                } else {
                    BOOST_THROW_EXCEPTION(std::runtime_error("adportab;e/json/extract<> exception"));
                }
            }
        }
        template<> ADPORTABLESHARED_EXPORT void extract( const boost::json::object& obj, boost::uuids::uuid& t, boost::json::string_view key );
    }

}
