// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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
#include <vector>
#include <boost/cstdint.hpp>
#include <boost/smart_ptr.hpp>

namespace adfs {

    class ostreambuf : public std::streambuf {
        size_t count_;
        size_t size_;
        size_t tail_;
        boost::int8_t * p_;
        void resize();
    public:
        ~ostreambuf();
        ostreambuf( std::size_t size = 0 );
        inline std::size_t size() const { return size_; }
        inline const boost::int8_t * p() const { return p_; }

        virtual int_type overflow ( int_type c ) {
            if ( count_ >= size_ )
                resize();
            p_[ count_++ - tail_ ] = c;
            return c;
        }
        virtual std::streamsize xsputn( const char * s, std::streamsize num );
    };

    class istreambuf : public std::streambuf {
        boost::int8_t * ptop_;
        const size_t size_;
    public:
        istreambuf( boost::int8_t *, size_t size );
    protected:
        virtual std::streambuf::int_type underflow();
    };

}


