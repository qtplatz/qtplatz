/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "quanresponses.hpp"
#include <boost/json.hpp>

using namespace adcontrols;

QuanResponses::QuanResponses()
{
}

QuanResponses::QuanResponses( const QuanResponses& t ) : values_( t.values_ )
{
}

QuanResponses&
QuanResponses::operator << (const QuanResponse& t )
{
    values_.push_back( t );
    return *this;
}

size_t
QuanResponses::size() const
{
    return values_.size();
}

void
QuanResponses::clear()
{
    values_.clear();
}

std::vector< QuanResponse >::iterator
QuanResponses::begin()
{
    return values_.begin();
}

std::vector< QuanResponse >::iterator
QuanResponses::end()
{
    return values_.end();
}

std::vector< QuanResponse >::const_iterator
QuanResponses::begin() const
{
    return values_.begin();
}

std::vector< QuanResponse >::const_iterator
QuanResponses::end() const
{
    return values_.end();
}

QuanResponses::operator boost::json::value () const
{
    boost::json::array a;
    for ( const auto& value: values_ )
        a.emplace_back( value );
    return a;
}
