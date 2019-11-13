/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "waveform_adder.hpp"
#include <adportable/debug.hpp>
#include <adportable/float.hpp>
#include <algorithm>
#include <numeric>
#include <cassert>

using namespace aqmd3controls;

waveform_adder::waveform_adder() : serialnumber_( 0 )
                                 , timeSinceEpoch_( 0 )
                                 , wellKnownEvents_(0)
                                 , reset_requested_( true )
{
}

void
waveform_adder::reset()
{
    reset_requested_ = true;
}

size_t
waveform_adder::add( const waveform& rhs )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    if ( waveform_ )
        reset_requested_ = adportable::compare< double >::approximatelyEqual( waveform_->xmeta().initialXOffset, rhs.xmeta().initialXOffset );

    if ( reset_requested_ || waveform_ == nullptr || ( rhs.size() != waveform_->size()) ) {

        waveform_ = std::make_shared< waveform >( rhs );

        reset_requested_ = false;

        serialnumber_0_ = rhs.serialnumber();
        timeSinceEpoch_0_ = rhs.epoch_time();
        wellKnownEvents_ = rhs.well_known_events();

    } else {
        *(waveform_) += rhs;
    }

    serialnumber_ = rhs.serialnumber();
    timeSinceEpoch_ = rhs.epoch_time();
    wellKnownEvents_ |= rhs.well_known_events();

    return waveform_->xmeta().actualAverages;
}

size_t
waveform_adder::actualAverage() const
{
    return waveform_->xmeta().actualAverages;
}

std::shared_ptr< waveform >
waveform_adder::fetch( bool reset )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    reset_requested_ = reset;

    if ( reset )
        return std::move( waveform_ );
    else
        return std::make_shared< waveform >( *waveform_ );
}
