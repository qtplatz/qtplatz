// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include <netcdf.h>
#include <cstdint>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <adportable/debug.hpp>

namespace adnetcdf {
    namespace netcdf {

        struct nat_t {};
        template< int NC_T > struct nc_type_t { typedef int value_type; };

        template<> struct nc_type_t< NC_NAT    > { typedef nat_t    value_type; value_type _; constexpr static const char * name = "nat";   }; // 0
        template<> struct nc_type_t< NC_BYTE   > { typedef int8_t   value_type; value_type _; constexpr static const char * name = "byte";  }; // 1
        template<> struct nc_type_t< NC_CHAR   > { typedef char     value_type; value_type _; constexpr static const char * name = "char";  }; // 2
        template<> struct nc_type_t< NC_SHORT  > { typedef int16_t  value_type; value_type _; constexpr static const char * name = "short"; }; // 3
        template<> struct nc_type_t< NC_INT    > { typedef int32_t  value_type; value_type _; constexpr static const char * name = "long";  }; // 4
        template<> struct nc_type_t< NC_FLOAT  > { typedef float    value_type; value_type _; constexpr static const char * name = "float"; }; // 5
        template<> struct nc_type_t< NC_DOUBLE > { typedef double   value_type; value_type _; constexpr static const char * name = "double";}; // 6
        template<> struct nc_type_t< NC_UBYTE  > { typedef uint8_t  value_type; value_type _; constexpr static const char * name = "uint8"; }; // 7
        template<> struct nc_type_t< NC_USHORT > { typedef uint16_t value_type; value_type _; constexpr static const char * name = "uint16";}; // 8
        template<> struct nc_type_t< NC_UINT   > { typedef uint32_t value_type; value_type _; constexpr static const char * name = "uint32";}; // 9
        template<> struct nc_type_t< NC_INT64  > { typedef int64_t  value_type; value_type _; constexpr static const char * name = "int64"; }; // 10
        template<> struct nc_type_t< NC_UINT64 > { typedef uint64_t value_type; value_type _; constexpr static const char * name = "uint64";}; // 11
        template<> struct nc_type_t< NC_STRING > { typedef char *   value_type; value_type _; constexpr static const char * name = "string";}; // 12

        typedef std::tuple< nc_type_t< NC_NAT >
                            , nc_type_t< NC_BYTE >
                            , nc_type_t< NC_CHAR   >
                            , nc_type_t< NC_SHORT  >
                            , nc_type_t< NC_INT    >
                            , nc_type_t< NC_FLOAT  >
                            , nc_type_t< NC_DOUBLE >
                            , nc_type_t< NC_UBYTE  >
                            , nc_type_t< NC_USHORT >
                            , nc_type_t< NC_UINT   >
                            , nc_type_t< NC_INT64  >
                            , nc_type_t< NC_UINT64 >
                            , nc_type_t< NC_STRING > > nc_types_t;

        struct null_t {};
        template < typename ... T >  struct type_list_t {};

        template< typename Last >  struct type_list_t< Last > {
            template< typename variant_type >
            variant_type do_cast( variant_type&&, int ) const {
                return {};
            }
        };

        template< typename First, typename... Args >  struct type_list_t< First, Args... > {
            template< typename variant_type >
            variant_type do_cast( variant_type&&, int typid ) const {
                if ( typid == 0 )
                    return First{};
                return type_list_t< Args... >{}.do_cast( variant_type{}, typid - 1 );
            }
        };

        template< typename Tuple > struct to_variant;

        template< typename... Args >
        struct to_variant< std::tuple< Args ... > > {
            using type = std::variant< Args ... >;
            type operator()( int typid ) const {  return type_list_t< Args ..., null_t >{}.do_cast( type{}, typid );  }
        };

        ///////////////////////////////////////////////////////////
        template< typename Tuple > struct to_variant_;

        template <typename... Ts>
        struct to_variant_< std::tuple<Ts...> >
        {
            using type = std::variant<std::monostate, typename Ts::value_type ...>;
        };

        //////////////////////////////////////////
        template< class Tuple, std::size_t... Is >
        std::string nc_type_name_impl( const Tuple& t, int typid, std::index_sequence<Is...> ) {
            std::string x;
            (( ( x = Is == typid ? std::get<Is>(Tuple{}).name : x),...));
            return x;
        }

        template< typename...  Args >
        std::string nc_type_name( int typid, const std::tuple< Args... >& t ) {
            return nc_type_name_impl( t, typid, std::index_sequence_for<Args...>{});
        };

    } // namespace netcdf
}
