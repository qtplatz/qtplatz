/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "tdcbase.hpp"
#include <adcontrols/countingmethod.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <vector>

using namespace acqrscontrols;

tdcbase::tdcbase() : tofChromatogramsMethod_( std::make_shared< adcontrols::TofChromatogramsMethod >() )
{
}

tdcbase::~tdcbase()
{
}

void
tdcbase::setCountingMethod( std::shared_ptr< const adcontrols::CountingMethod > m )
{
    countingMethod_ = m;
}

std::shared_ptr< const adcontrols::CountingMethod >
tdcbase::countingMethod() const
{
    return countingMethod_;
}

bool
tdcbase::setTofChromatogramsMethod( const adcontrols::TofChromatogramsMethod& m )
{
    tofChromatogramsMethod_ = std::make_shared< adcontrols::TofChromatogramsMethod >( m );
    return true;
}

std::shared_ptr< const adcontrols::TofChromatogramsMethod >
tdcbase::tofChromatogramsMethod() const
{
    return tofChromatogramsMethod_;
}

void
tdcbase::eraseTofChromatogramsMethod()
{
    return tofChromatogramsMethod_.reset();
}

bool
tdcbase::computeCountRate( const adcontrols::TimeDigitalHistogram& histogram
                           , const adcontrols::CountingMethod& cm
                           , std::vector< std::pair< size_t, size_t > >& rates )
{
    if ( ! cm.enable() )
        return false;

    using adcontrols::CountingMethod;

    if ( rates.size() != cm.size() )
        rates.resize( cm.size() );

    int idx(0);
    for ( auto& v : cm ) {
        if ( std::get< CountingMethod::CountingEnable >( v ) ) {

            auto range = std::get< CountingMethod::CountingRange >( v );

            rates[ idx ].first += histogram.accumulate( range.first, range.second );
            rates[ idx ].second += histogram.trigger_count();
        }
        idx++;
    }
    return true;
    // return ap240::tdcdoc::computeCountRate( histogram, m, v );
}
