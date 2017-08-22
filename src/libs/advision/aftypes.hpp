/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <af/defines.h>
#include <complex>
#include <tuple>
#include <utility>

namespace advision {

    typedef std::tuple< float                      // f32
                        , std::complex<float>      // c32
                        , double                   // f64
                        , std::complex<double>     // c64
                        , bool                     // b8
                        , int32_t                  // s32
                        , uint32_t                 // u32
                        , uint8_t                  // u8
                        , int64_t                  // s64
                        , uint64_t                 // u64
                        , int16_t                  // s16
                        , uint16_t                 // u16
                        > af_array_types;
    
    template < class... Args > struct af_type_list {
        template < std::size_t N >
        using type = typename std::tuple_element<N, std::tuple<Args...> >::type;
    };
    
    template <class T, class Tuple>
    struct af_type_index;
    
    template <class T, class... Types>
    struct af_type_index<T, std::tuple<T, Types...> > {
        static const af_dtype value = af_dtype(0);
    };
    
    template <class T, class U, class... Types>
    struct af_type_index<T, std::tuple<U, Types...> > {
        static const af_dtype value = af_dtype( 1 + af_type_index<T, std::tuple<Types...> >::value );
    };
    
    // usage: af_array_type< 0 >::type := 'float'
    template< af_dtype N > using af_type = std::tuple_element< N, af_array_types >;
    
    // usage: std::cout << af_type_value< float >::value << std::endl;
    template< class T > using af_type_value = af_type_index< T, af_array_types >;
    
} // namespace advision
