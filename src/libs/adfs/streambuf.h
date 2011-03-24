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
#include <boost/smart_ptr.hpp>
#include <vector>

namespace adfs {

    class streambuf : public std::streambuf {
        size_t count_;
        size_t size_;
        size_t tail_;
        unsigned char * p_;
        std::vector< boost::shared_array<unsigned char> > vec_;

        void resize();
    public:
        ~streambuf();
        streambuf( std::size_t size = 0 );
        const std::vector< boost::shared_array<unsigned char> >& vec() const { return vec_; }

        virtual int_type overflow ( int_type c ) {
            if ( count_ >= size_ )
                resize();
            p_[ count_++ - tail_ ] = c;
            return c;
        }
        virtual std::streamsize xsputn( const char * s, std::streamsize num );
    };

}


