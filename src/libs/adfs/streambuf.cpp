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

#include "streambuf.h"

using namespace adfs;
static const size_t unit_size = 1024 * 64;

streambuf::~streambuf()
{
    delete [] p_;
}

streambuf::streambuf( std::size_t size ) : count_(0), size_(size), tail_(0), p_(0)
{
    resize();
}

void
streambuf::resize()
{
    tail_ = size_;
    size_ += unit_size; // 64k per page
    
    boost::shared_array< unsigned char > p( new unsigned char [ unit_size ] );
    vec_.push_back( p );
    p_ = vec_.back().get();
}

std::streamsize
streambuf::xsputn( const char * s, std::streamsize num )
{
    for ( int i = 0; i < num; ++i ) {
        if ( count_ >= size_ )
            resize();
        p_[ count_++ - tail_ ] = *s++;
    }
    return num;
}
