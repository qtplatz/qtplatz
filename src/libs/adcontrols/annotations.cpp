/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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

#include "annotations.hpp"
#include <adportable/debug.hpp>
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

annotations&
annotations::operator << ( const annotation& t )
{
    auto it = std::find_if( vec_.begin(), vec_.end(), [&](const auto& a){ return a.index() == t.index() && a.dataFormat() == t.dataFormat(); });
    if ( it != vec_.end() )
        *it = t;
    else
        vec_.emplace_back( t );
    return *this;
}

annotations&
annotations::operator << ( annotation&& t )
{
    auto it = std::find_if( vec_.begin(), vec_.end(), [&](const auto& a){ return a.index() == t.index() && a.dataFormat() == t.dataFormat(); });
    if ( it != vec_.end() )
        *it = std::move(t);
    else
        vec_.emplace_back( t );
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
		std::sort( vec_.begin(), vec_.end(), []( const annotation& a, const annotation& b ){ return a.priority() > b.priority(); } );
    } else if ( item == Index ) {
		std::sort( vec_.begin(), vec_.end(), []( const annotation& a, const annotation& b ){ return a.index() > b.index(); } );
    }
}
