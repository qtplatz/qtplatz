/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef STREAMBUF_HPP
#define STREAMBUF_HPP

#include <streambuf>
#include <vector>
#include "adsequence_global.hpp"

namespace adsequence {

    typedef char char_t;

    class ADSEQUENCESHARED_EXPORT streambuf : public std::basic_streambuf<char_t> {
        std::vector<char_t>& vec_;
    public:
        ~streambuf() {}
        streambuf( std::vector<char_t>& t ) : vec_( t ) {
            setg( &vec_[ 0 ], &vec_[ 0 ], &vec_[0] + vec_.size() );
        }
    protected:
        virtual std::streamsize xsputn( const char * s, std::streamsize num );
        virtual std::basic_streambuf<char_t>::int_type overflow( int_type c );
        virtual std::basic_streambuf<char_t>::int_type underflow();
    };

}

#endif // STREAMBUF_HPP
