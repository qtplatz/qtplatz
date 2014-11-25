/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "mspeaks.hpp"
#include "mspeak.hpp"

using namespace adcontrols;

MSPeaks::~MSPeaks()
{
}

MSPeaks::MSPeaks()
{
}

MSPeaks::MSPeaks( const MSPeaks& t ) : vec_( t.vec_ )
{
}

const std::vector<double>&
MSPeaks::x() const
{
    return x_;
}

const std::vector<double>&
MSPeaks::y() const
{
    return y_;
}

const std::vector<double>&
MSPeaks::coeffs() const
{
    return coeffs_;
}

void
MSPeaks::polinomials( const std::vector<double>& x, const std::vector<double>& y, const std::vector<double>& coeffs)
{
    x_ = x;
    y_ = y;
    coeffs_ = coeffs;
}

const MSPeak&
MSPeaks::operator [] ( size_t idx ) const
{
    return vec_[ idx ];
}

void
MSPeaks::clear()
{
    vec_.clear();
}

size_t
MSPeaks::size() const
{
    return vec_.size();
}

MSPeaks::iterator_type
MSPeaks::begin()
{
    return vec_.begin();
}

MSPeaks::iterator_type
MSPeaks::end()
{ 
    return vec_.end();
}

MSPeaks::const_iterator_type
MSPeaks::begin() const
{
    return vec_.begin();
}

MSPeaks::const_iterator_type
MSPeaks::end() const
{
    return vec_.end();
}

MSPeaks::iterator_type
MSPeaks::erase( iterator_type it )
{
    return vec_.erase( it );
}

MSPeaks::iterator_type
MSPeaks::erase( iterator_type first, iterator_type last )
{
    return vec_.erase( first, last );
}

MSPeaks&
MSPeaks::operator << ( const MSPeak& t )
{
    vec_.push_back( t );
	return *this;
}

