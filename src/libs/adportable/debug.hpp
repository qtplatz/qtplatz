// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <sstream>
#include "adportable_global.h"

namespace adportable {
    class ADPORTABLESHARED_EXPORT debug;
}

namespace boost {
    namespace system { class error_code; }
    namespace property_tree {
        template< class Key, class Data, class KeyCompare > class basic_ptree;
        typedef basic_ptree< std::string, std::string, std::less< std::string > > ptree;
    }
}

namespace adportable {

    class debug {
        std::ostringstream o_;
        std::string file_;
        int line_;
    public:
        debug(const char * file = 0, const int line = 0);
        ~debug(void);
        static void initialize( const std::string& filename );
        std::string where() const;
        std::string str() const { return o_.str(); }

        template<typename T> debug& operator << ( const T& t ) { o_ << t; return *this; }

        template<typename F, typename S> debug& operator << ( const std::pair<F,S>& t ){
            (*this) << "{" << t.first << ", " << t.second << "}";
            return *this;
        }
		debug& operator << ( const wchar_t *);
		debug& operator << ( const std::wstring& t );
    };

    template<> ADPORTABLESHARED_EXPORT debug& debug::operator << ( const std::wstring& t );
    template<> ADPORTABLESHARED_EXPORT debug& debug::operator << ( const boost::system::error_code& );
    template<> ADPORTABLESHARED_EXPORT debug& debug::operator << ( const boost::property_tree::ptree& );

    inline std::string where( const char * file, const int line ) {
        debug x( file, line );
        return x.where();
    }
}

#define ADDEBUG() adportable::debug(__FILE__, __LINE__)
