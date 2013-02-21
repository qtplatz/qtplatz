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

#include "annotations.hpp"
#include <boost/bind.hpp>
#include <algorithm>

using namespace adcontrols;

annotations::annotations()
{
}

annotations::annotations( const annotations& t ) : vec_( t.vec_ )
{
}

size_t
annotations::size() const
{
    return vec_.size();
}

bool
annotations::empty() const
{
    return vec_.empty();
}

void
annotations::clear()
{
    vec_.clear();
}

annotations::operator const annotations::vector_type& () const
{
    return vec_;
}

annotations&
annotations::operator << ( const annotation& t )
{
    vec_.push_back( t );
    return *this;
}

const annotation&
annotations::operator [] ( size_t idx ) const
{
    return vec_[ idx ];
}

annotation&
annotations::operator [] ( size_t idx )
{
    return vec_[ idx ];
}

void
annotations::sort( enum OrderBy item )
{
    if ( item == Priority ) {
        std::sort( vec_.begin(), vec_.end()
                   , boost::bind( &annotation::priority, _1 ) < boost::bind( &annotation::priority, _2 ) );
        
    } else if ( item == Index ) {
        std::sort( vec_.begin(), vec_.end()
                   , boost::bind( &annotation::index, _1 ) < boost::bind( &annotation::index, _2 ) );

    }
}
