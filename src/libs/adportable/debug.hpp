// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC
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
#include "adportable_global.h"
#include <sstream>
#include <exception>
#include <utility>
#include <tuple>

namespace adportable {
    class ADPORTABLESHARED_EXPORT debug;
}

namespace boost {
    class exception;
}

namespace adportable {

    class debug {
        std::ostringstream o_;
        std::string file_;
        int line_;

        template<class Tuple, std::size_t... Is>
        void debug_tuple_impl(const Tuple& t, std::index_sequence<Is...>){
            (((*this) << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
        }
    public:
        debug(const char * file = 0, const int line = 0);
        ~debug(void);
        static void initialize( const std::string& filename );
        std::string where() const;
        std::string str() const;

        debug& operator<<( std::ostream&(*f)(std::ostream&) ) { o_ << f; return *this; } // handle std::endl

        template<typename T> debug& operator << ( const T& t ) { o_ << t; return *this; }

        template<typename F, typename S> debug& operator << ( const std::pair<F,S>& t ){
            return (*this) << "{" << t.first << ", " << t.second << "}";
        }

        template<typename... Args> debug& operator << ( const std::tuple< Args...>& t ) {
            (*this) << "{";
            debug_tuple_impl( t, std::index_sequence_for<Args...>{});
            return (*this) << "}";
        }

		debug& operator << ( const wchar_t *);
		debug& operator << ( const std::wstring& t );
    };

    template<> ADPORTABLESHARED_EXPORT debug& debug::operator << ( const std::wstring& t );
    template<> ADPORTABLESHARED_EXPORT debug& debug::operator << ( const std::error_code& );
    template<> ADPORTABLESHARED_EXPORT debug& debug::operator << ( const std::exception& );
    template<> ADPORTABLESHARED_EXPORT debug& debug::operator << ( const boost::exception& );

    inline std::string where( const char * file, const int line ) {
        debug x( file, line );
        return x.where();
    }
}

#define ADDEBUG() adportable::debug(__FILE__, __LINE__)
