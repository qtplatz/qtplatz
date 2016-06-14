// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include <boost/any.hpp>

namespace adportable {

    template<class T> struct a_type {

        static bool is_a( boost::any& a ) {
#if defined __GNUC__ 
            // See issue on boost.  https://svn.boost.org/trac/boost/ticket/754
            return std::strcmp( a.type().name(), typeid( T ).name() ) == 0;
#else
            return a.type() == typeid( T );
#endif
        }

        static bool is_a( const boost::any& a ) {
#if defined __GNUC__ 
            // See issue on boost.  https://svn.boost.org/trac/boost/ticket/754
            return std::strcmp( a.type().name(), typeid( T ).name() ) == 0;
#else
            return a.type() == typeid( T );
#endif
        }
        
        static bool is_pointer( boost::any& a ) {
            return a_type< T* >::is_a( a );
        }
        
        static bool is_const_pointer( boost::any& a ) {
            return a_type< const T* >::is_a( a );
        }
    };

}
