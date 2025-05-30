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

#include <opencv2/core/core.hpp>
#include <tuple>
#include <utility>

namespace advision { // namespace cv {

    enum cv_channels { C1 = 00, C2 = 010, C3 = 020, C4 = 030 };
    enum cv_types { _8U = 00, _8S = 01, _16U = 02, _16S = 03, _32S = 04, _32F = 05, _64F = 06 };

    typedef std::tuple<  uint8_t, int8_t, uint16_t, int16_t, int32_t, float, double > cv_mat_types;

    template < class... Args > struct type_list {
        template < std::size_t N >
        using type = typename std::tuple_element<N, std::tuple<Args...> >::type;
    };

    template <class T, class Tuple>
    struct type_index;
        
    template <class T, class... Types>
    struct type_index<T, std::tuple<T, Types...> > {
        static const std::size_t value = 0;
    };
        
    template <class T, class U, class... Types>
    struct type_index<T, std::tuple<U, Types...> > {
        static const std::size_t value = 1 + type_index<T, std::tuple<Types...> >::value;
    };

    // type from index
    // usage: cv_mat_type< 5 >::type  // := 'float'
    template< size_t N > using cv_type = std::tuple_element< N, cv_mat_types >;

    // index from type
    // usage: std::cout << cv_type_value< float >::value << std::endl;
    template< class T > using cv_type_value = type_index< T, cv_mat_types >;

        
    /////////// extended cv::Mat ///////////////
    template< typename T = uint8_t, uint cn = 1 > class mat_ : public ::cv::Mat {
    public:
        typedef T value_type;
        typedef ::cv::Vec<T,cn> VecT;
        enum { type_value = cv_type_value<T>::value | ((cn - 1) << 3) }; // compile time mat.type()
        enum { depth = cn };
            
        mat_( size_t rows = 0, size_t cols = 0 )
            : ::cv::Mat( rows, cols, cv_type_value<T>::value | ((cn - 1) << 3) ) {
        }

        mat_( const mat_& t ) : ::cv::Mat( t ) {
        }

        mat_& operator = ( const ::cv::Mat& m ) {
            assert( m.type() == type_value );
            static_cast<::cv::Mat&>(*this) = m;
            return *this;
        }

    };
}

