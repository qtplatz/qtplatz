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

#include "countingresult.hpp"
#include "constants.hpp"

using namespace adcontrols;

CountingResult::CountingResult()
{
}

CountingResult::CountingResult( const CountingResult& t ) : values_( t.values_ )
{
}

CountingResult&
CountingResult::operator = ( const CountingResult& rhs )
{
    values_ = rhs.values_;
	return *this;
}

size_t
CountingResult::size() const
{
    return values_.size();
}

void
CountingResult::clear()
{
    values_.clear();
}

CountingResult::value_type&
CountingResult::operator [] ( size_t idx )
{
    return values_[ idx ];
}

const CountingResult::value_type&
CountingResult::operator [] ( size_t idx ) const
{
    return values_[ idx ];
}

CountingResult::iterator
CountingResult::begin()
{
    return values_.begin();
}

CountingResult::iterator
CountingResult::end()
{
    return values_.end();
}

CountingResult::const_iterator
CountingResult::begin() const
{
    return values_.begin();
}

CountingResult::const_iterator
CountingResult::end() const
{
    return values_.end();
}

