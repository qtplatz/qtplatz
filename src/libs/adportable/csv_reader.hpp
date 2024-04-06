// -*- C++ -*-
/**************************************************************************
**
** MIT License
** Copyright (c) 2021-2022 Toshinobu Hondo, Ph.D

** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:

** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**************************************************************************/

#pragma once

//#if not defined WIN32 // MSVC++ 14 can not compile spirit::x3

#include "adportable_global.h"
#include "debug.hpp"
#include <boost/variant.hpp>
#include <boost/spirit/home/x3.hpp>
#include <memory>
#include <vector>

namespace adportable {
    namespace csv {

        namespace x3 = boost::spirit::x3;

        typedef boost::variant< x3::unused_type, std::string, double, int > variant_type;
        typedef std::vector< variant_type > list_type;

        class ADPORTABLESHARED_EXPORT csv_reader;

        class csv_reader {
            csv_reader( const csv_reader& ) = delete;
            csv_reader& operator = ( const csv_reader& ) = delete;
        public:
            ~csv_reader();
            csv_reader();
            csv_reader( const std::string& file );
            csv_reader( std::ifstream&& );
            void rewind();
            bool skip( size_t nlines );
            bool skip( std::istream&, size_t nlines );
            bool read( list_type& );
            bool read( std::istream&, list_type& );
        private:
            class impl;
            std::unique_ptr< impl > impl_;
        };

        //////////////////
        // convert list_type to fully typed tuple
        // type T must be POD type (std::string is not a POD type though, it is supported)

        template< typename T >
        struct to_value {
            template< typename V > T operator()( const V& v ) const { return v; }
        };
        // std::string
        template<> template<typename V> std::string to_value<std::string>::operator()(const V& v) const { return std::to_string( v ); }
        // int|double
        template<> template<> int to_value<int>::operator()(const std::string& v) const { try { return stoi(v); } catch ( ... ){}; return {}; }
        template<> template<> double to_value<double>::operator()(const std::string& v) const { try { return stod(v); } catch ( ... ){}; return {}; }

        // visitor
        template< typename T >
        struct value_to : boost::static_visitor < T > {
            T operator()( const boost::spirit::x3::unused_type& v ) const { return {}; }
            template<typename V> T operator()( V& v ) const {  return to_value< T >()( v ); }
        };

        template< typename Tuple, std::size_t... Is>
        Tuple to_tuple_impl( const list_type& list, Tuple&& t, std::index_sequence<Is...> )
        {
            (( std::get<Is>(t) = boost::apply_visitor( value_to< std::tuple_element_t<Is, Tuple> >(), list[Is] ) ), ...);
            return t;
        }

        template<class... Args>
        std::tuple<Args...>
        to_tuple( const list_type& list )
        {
            return to_tuple_impl( list, std::tuple<Args...>{}, std::index_sequence_for<Args...>{});
        }
    }
}
//#endif
