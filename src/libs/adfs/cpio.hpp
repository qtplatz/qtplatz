// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include <streambuf>
#include <boost/smart_ptr.hpp>
#include <adfs/folium.hpp>

namespace adfs {

    class folium;
    typedef char char_t;

    namespace detail { 
        class cpio : public std::basic_streambuf<char_t> {
            boost::scoped_array< char_t > p_;
            size_t size_;
            size_t count_;
            bool resize( size_t );
        public:
            cpio() : size_(0), count_(0) {}
            cpio( size_t size, char_t * p = 0 );

            inline size_t size() const { return count_; }
            inline const char_t * get() const { return p_.get(); }
            inline char_t * get() { return p_.get(); }
        protected:
            virtual std::streamsize xsputn( const char_t * s, std::streamsize num );
            virtual std::basic_streambuf<char_t>::int_type overflow ( int_type c );
            virtual std::basic_streambuf<char_t>::int_type underflow();
        };
    };

    template<class archive_type> class cpio {
    public:

        template<class T> static bool copyin( const T& t, detail::cpio& obuf ) {
            std::ostream os( &obuf );
            return archive_type::archive( os, t );
        }

        template<class T> static bool copyout( T& t, detail::cpio& ibuf ) {
            std::istream is( &ibuf );
            return archive_type::restore( is, t );
        }

        template<class T> static bool copyin( const T& t, adfs::folium& f ) {
            detail::cpio obuf;
            std::ostream os( &obuf );
            return archive_type::archive( os, t ) && f.write( obuf.size(), obuf.get() );
        }

        template<class T> static bool copyout( T& t, adfs::folium& f ) {
            detail::cpio ibuf( f.size() );
            std::istream is( &ibuf );
            return f.read( ibuf.size(), ibuf.get() ) && archive_type::restore( is, t );
        }

    };

}
