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
#include <boost/variant.hpp>
#include <boost/spirit/home/x3.hpp>
#include "csv_reader.hpp"
#include <sstream>
#include <iomanip>

namespace adportable {
    namespace csv {
        struct string_visitor : boost::static_visitor<std::string> {
            bool quoted_;
            string_visitor( bool quoted = false ) : quoted_( quoted ) {}
            std::string operator()( const auto& v ) const {
                return std::to_string(v);
            }
            std::string operator()( const std::string& v ) const {
                if ( quoted_ ) {
                    std::stringstream o;
                    o << std::quoted( v );
                    return o.str();
                } else {
                    return v;
                }
            }
            std::string operator()( const boost::spirit::x3::unused_type& ) const {
                return {};
            }
        };
        // std::string make_csv_string( const list_string_type& list );
    }
}
//#endif
