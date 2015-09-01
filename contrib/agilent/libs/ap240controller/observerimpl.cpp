/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "observerimpl.hpp"

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <algorithm>

using namespace ap240controller;
    
ObserverImpl::ObserverImpl() : uuid_( boost::uuids::name_generator( base_uuid() )( "1.ap240.ms-cheminfo.com" ) )
{
    so::Description desc;
    desc.set_trace_method( so::eTRACE_IMAGE_TDC );
    desc.set_spectrometer( so::eMassSpectrometer );
    desc.set_trace_id( "AP240.WAVEFORM" );  // unique name for the trace, can be used as 'data storage name'
    desc.set_trace_display_name( "AP240 Waveforms" );
    desc.set_axis_label( so::Description::axisX, "Time" );
    desc.set_axis_label( so::Description::axisY, "mV" );
    desc.set_axis_decimals( so::Description::axisX, 3 );
    desc.set_axis_decimals( so::Description::axisY, 3 );
    setDescription( desc );
}

ObserverImpl::~ObserverImpl()
{
}

const boost::uuids::uuid&
ObserverImpl::uuid() const
{
    return uuid_;
}

uint64_t 
ObserverImpl::uptime() const 
{
    return 0;
}

void 
ObserverImpl::uptime_range( uint64_t& oldest, uint64_t& newest ) const 
{
    oldest = newest = 0;
    
    std::lock_guard< std::mutex > lock( const_cast<ObserverImpl *>( this )->mutex() );

    if ( ! que_.empty() ) {
        oldest = que_.front()->pos();
        newest = que_.back()->pos();
    }
    
}

std::shared_ptr< so::DataReadBuffer >
ObserverImpl::readData( uint32_t pos )
{
    std::lock_guard< std::mutex > lock( mutex() );
    
    if ( que_.empty() )
        return 0;
    
    if ( pos == std::numeric_limits<uint32_t>::max() ) {
        return que_.back();
    }

    auto it = std::find_if( que_.begin(), que_.end(), [pos]( const std::shared_ptr< so::DataReadBuffer >& p ){ return pos == p->pos(); } );
    if ( it != que_.end() )
        return *it;
    
    return 0;
}

int32_t
ObserverImpl::posFromTime( uint64_t usec ) const 
{
    std::lock_guard< std::mutex > lock( const_cast< ObserverImpl *>(this)->mutex() );
    
    if ( que_.empty() )
        return false;

    auto it = std::lower_bound( que_.begin(), que_.end(), usec
                                , [] ( const std::shared_ptr< so::DataReadBuffer >& p, uint64_t usec ) { return p->timepoint() < usec; } );
    if ( it != que_.end() )
        return (*it)->pos();

    return 0;
}

//
void
ObserverImpl::operator << ( std::shared_ptr< so::DataReadBuffer >& t )
{
    std::lock_guard< std::mutex > lock( mutex() );

    que_.push_back( t );

    if ( que_.size() > 4096 )
        que_.erase( que_.begin(), que_.begin() + 1024 );
}
