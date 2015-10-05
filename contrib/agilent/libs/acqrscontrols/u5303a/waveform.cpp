/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "waveform.hpp"
#include "threshold_result.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
#include <adportable/serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adicontroller/signalobserver.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/variant.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

using namespace acqrscontrols::u5303a;

waveform::waveform( std::shared_ptr< identify >& id ) : serialnumber_( 0 )
                                                      , wellKnownEvents_( 0 )
                                                      , timeSinceEpoch_( 0 )
                                                      , ident_( id )
{
}


const int32_t *
waveform::trim( metadata& meta, uint32_t& nSamples ) const
{
    meta = meta_;

    size_t offset = 0;
    if ( method_.digitizer_delay_to_first_sample < method_.delay_to_first_sample_ )
        offset = size_t( ( ( method_.delay_to_first_sample_ - method_.digitizer_delay_to_first_sample ) / meta.xIncrement ) + 0.5 );

    nSamples = method_.nbr_of_s_to_acquire_;
    if ( nSamples + offset > method_.digitizer_nbr_of_s_to_acquire )
        nSamples = uint32_t( method_.digitizer_nbr_of_s_to_acquire - offset );

    meta.initialXOffset = method_.delay_to_first_sample_;
    meta.actualPoints = nSamples;

    return d_.data() + offset;
}

std::pair<double, int>
waveform::operator [] ( size_t idx ) const
{
    double time = idx * meta_.xIncrement + meta_.initialXOffset;
    return std::make_pair( time, d_[ idx ] );
    
    // switch( meta_.dataType ) {
    // case 1: return std::make_pair( time, *(begin<int8_t>()  + idx) );
    // case 2: return std::make_pair( time, *(begin<int16_t>() + idx) );
    // case 4: return std::make_pair( time, *(begin<int32_t>() + idx) );
    // }
    
    //throw std::exception();
}
