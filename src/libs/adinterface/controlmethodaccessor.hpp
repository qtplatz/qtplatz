/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include <string>
#include <boost/any.hpp>
#include <adinterface/controlmethodC.h>
#include <adinterface/controlmethodhelper.hpp>

namespace adinterface {

    template<class T, class C> class ControlMethodAccessorT {
        std::wstring instId_;
        unsigned int unitnumber_;
    public:
        ControlMethodAccessorT( const std::wstring& instId
                                , unsigned int unitnumber = 0 ) : instId_( instId )
                                                                , unitnumber_( unitnumber ) {
        }
        
        bool getContents( const T& t, boost::any& a ) const {
#if defined __GNUC__ 
            // See issue on boost.  https://svn.boost.org/trac/boost/ticket/754
            if ( std::strcmp( a.type().name(), typeid( ::ControlMethod::Method * ).name() ) != 0 )
                return false;
#else
            if ( a.type() != typeid( ::ControlMethod::Method * ) )
                return false;
#endif
            ::ControlMethod::Method * m = boost::any_cast< ::ControlMethod::Method * >( a );
            if ( m ) {
                ::ControlMethod::MethodLine * line = ControlMethodHelper::findFirst( *m, instId_, unitnumber_ );
                if ( line ) {
                    C * p = 0;
                    if ( line->data >>= p )
                        t.getMethod( * p );
                } else {
                    C c;
                    t.getMethod( c );
                    ::ControlMethod::MethodLine line;
                    line.data <<= c;
                    ControlMethodHelper::append( *m, line, instId_, unitnumber_ );
                }
            }
            return true;
        }

        bool setContents( T& t, boost::any& a ) {
#if defined __GNUC__ 
            // See issue on boost.  https://svn.boost.org/trac/boost/ticket/754
            if ( std::strcmp( a.type().name(), typeid( ::ControlMethod::Method ).name() ) != 0 )
                return false;
#else
            if ( a.type() != typeid( ::ControlMethod::Method ) )
                return false;
#endif
            const ::ControlMethod::Method& m = boost::any_cast< const ::ControlMethod::Method& >( a );
            const ::ControlMethod::MethodLine * line = ControlMethodHelper::findFirst( m, instId_, unitnumber_ );
            if ( line ) {
                const C * p = 0;
                if ( line->data >>= p )
                    t.setMethod( *p );
            }
            return true;
        }
    };

}

