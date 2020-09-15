/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "traceobserver.hpp"
#include "advalue.hpp"
#include <adportable/debug.hpp>
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/version.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <algorithm>
#include <sstream>

using namespace socfpga::dgmod;

const constexpr boost::uuids::uuid TraceObserver::__objid__;

TraceObserver::TraceObserver() : elapsed_time_at_inject_(0)
                               , epoch_time_at_inject_(0)
                               , adc_counter_at_inject_(0)
{
    so::Description desc;
    desc.set_trace_method( so::eTRACE_TRACE );
    desc.set_spectrometer( so::eUnknownSpectrometer );  // ADC input
    desc.set_trace_id( __objtext__ );                   // unique name for the trace, can be used as 'data storage name'
    desc.set_trace_display_name( L"ADC" );
    desc.set_axis_label( so::Description::axisX, L"Time" );
    desc.set_axis_label( so::Description::axisY, L"mV" );
    desc.set_axis_decimals( so::Description::axisX, 3 );
    desc.set_axis_decimals( so::Description::axisY, 3 );
    desc.set_objid( __objid__ );
    desc.set_objtext( __objtext__ );
    setDescription( desc );
}

TraceObserver::~TraceObserver()
{
}

uint64_t
TraceObserver::uptime() const
{
    return 0;
}

void
TraceObserver::uptime_range( uint64_t& oldest, uint64_t& newest ) const
{
    oldest = newest = 0;

    std::lock_guard< std::mutex > lock( const_cast<TraceObserver *>( this )->mutex() );

    if ( !que_.empty() ) {
        oldest = que_.front()->timepoint(); //waveform_pair::pos( que_.front() );
        newest = que_.back()->timepoint();  //waveform_pair::pos( que_.back() );
    }

}

std::shared_ptr< so::DataReadBuffer >
TraceObserver::readData( uint32_t pos )
{
    std::lock_guard< std::mutex > lock( mutex() );

    if ( que_.empty() )
        return 0;

    if ( pos == std::numeric_limits<uint32_t>::max() ) {
        return que_.back();
    }

    auto it = std::lower_bound( que_.begin(), que_.end(), pos
                                , []( const std::shared_ptr<so::DataReadBuffer>& p, uint32_t pos ){
                                      return p->pos() < pos;
                                  });

    if ( it != que_.end() ) {
        if ( (*it)->pos() == pos )
            return (*it);
    }

    ADDEBUG() << "readData( " << pos << " ) failed."
              << " begin =" << que_.front()->pos() << " end = " << que_.back()->pos();
    // std::for_each( que_.begin(), que_.end(), [](auto& p){ ADDEBUG() << p->pos(); } );

    return 0;
}

int32_t
TraceObserver::posFromTime( uint64_t nsec ) const
{
    std::lock_guard< std::mutex > lock( const_cast< TraceObserver *>(this)->mutex() );

    if ( que_.empty() )
        return false;

    auto it = std::lower_bound( que_.begin(), que_.end(), nsec
                                , [] ( const std::shared_ptr< so::DataReadBuffer >& p, uint64_t usec ) { return p->timepoint() < usec; } );

    if ( it != que_.end() )
        return (*it)->pos(); //waveform_pair::pos(*it);

    return 0;
}

uint32_t
TraceObserver::emplace_back( std::vector< advalue >&& values, uint32_t events )
{
    if ( values.empty() )
        return 0;

    auto rb = std::make_shared< so::DataReadBuffer >();

    const auto& top    = values[0];

    auto pos = top.adc_counter;

    rb->pos()          = top.adc_counter;
    rb->timepoint()    = top.elapsed_time;  // timepoint should be a steady time.  elapsed_time of this device is fpga clock counts that is ok
    rb->elapsed_time() = top.elapsed_time;
    rb->epoch_time()   = top.posix_time;
    rb->fcn()          = 0;
    rb->ndata()        = values.size();
    rb->events()       = events;
    //rb->setData( std::move( values ) );
    rb->setData( std::make_shared< std::vector< advalue > >( std::move( values ) ) );

    std::lock_guard< std::mutex > lock( mutex() );
    if ( que_.size() > 200 ) {
        auto tail = que_.begin();
        std::advance( tail, 50 );
        que_.erase( que_.begin(), tail );
    }

    que_.emplace_back( std::move( rb ) );

    return pos;
}

void
TraceObserver::setInjectData( const advalue& value )
{
    elapsed_time_at_inject_ = value.elapsed_time;
    epoch_time_at_inject_ = value.elapsed_time;
    adc_counter_at_inject_ = value.adc_counter;
}
