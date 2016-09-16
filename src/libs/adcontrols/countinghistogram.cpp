// This is a -*- C++ -*- header.
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

#include "countinghistogram.hpp"
#include "countingdata.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace adcontrols;

CountingHistogram::CountingHistogram() : xIncrement_( 1.0e-9 )
{
}

CountingHistogram::CountingHistogram( const CountingHistogram& t ) : xIncrement_( t.xIncrement_ )
                                                                   , pklists_( t.pklists_ )
{
}

CountingHistogram&
CountingHistogram::operator = ( const CountingHistogram& t )
{
    xIncrement_ = t.xIncrement_;
    pklists_ = t.pklists_;
    return *this;
}

CountingHistogram&
CountingHistogram::operator << ( const CountingData& data )
{
    if ( data.peaks().size() ) {

        for ( auto& pk: data.peaks() ) {
            
            auto it = std::lower_bound( pklists_.begin(), pklists_.end(), pk
                                        , [&]( const std::pair< double, std::vector< adcontrols::CountingPeak > >& a
                                              , const adcontrols::CountingPeak& b ){
                                            return size_t( 0.5 + a.first / xIncrement_ ) < size_t( 0.5 + b.apex().first / xIncrement_ );
                                        });

            if ( it == pklists_.end() ) {
                
                pklists_.emplace_back( size_t( 0.5 + pk.apex().first / xIncrement_ ) * xIncrement_
                                       , std::vector< adcontrols::CountingPeak >( { pk } ) );
                
            } else if ( size_t( 0.5 + it->first / xIncrement_ ) ==
                        size_t( 0.5 + std::get< CountingPeak::pk_apex >( pk.d_ ).first / xIncrement_ ) ) {

                it->second.emplace_back( pk );
            
            } else {

                pklists_.emplace( it, std::make_pair( pk.apex().first, std::vector< adcontrols::CountingPeak >( { pk } ) ) );

            }
        }
    }
    return *this;
}

void
CountingHistogram::setXIncrement( double x )
{
    xIncrement_ = x;
}

double
CountingHistogram::xIncrement() const
{
    return xIncrement_;
}

CountingHistogram::const_iterator
CountingHistogram::begin() const
{
    return pklists_.begin();
}

CountingHistogram::const_iterator
CountingHistogram::end() const
{
    return pklists_.end();
}

CountingHistogram::iterator
CountingHistogram::begin()
{
    return pklists_.begin();
}

CountingHistogram::iterator
CountingHistogram::end()
{
    return pklists_.end();
}

void
CountingHistogram::clear()
{
    pklists_.clear();
}


size_t
CountingHistogram::size() const
{
    return pklists_.size();
}
