/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include "waveformobserver.hpp"
#include <acqrscontrols/constants.hpp>
#include <acqrscontrols/acqiris_waveform.hpp>
#include <adportable/debug.hpp>
#include <boost/version.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <adportable/portable_binary_oarchive.hpp>

#include <algorithm>
#include <sstream>

using namespace aqdrv4controller;
static const char * objtext__ = acqrscontrols::dc122::waveform_observer_name; // "1.dc122.ms-cheminfo.com";

//"{04c23c3c-7fd6-11e6-aa18-b7efcbc41dcd}"
WaveformObserver::WaveformObserver()
    : objid_( boost::uuids::string_generator()( acqrscontrols::dc122::waveform_observer_uuid ) )
{
    so::Description desc;
    desc.set_trace_method( so::eTRACE_SPECTRA );
    desc.set_spectrometer( so::eMassSpectrometer );
    desc.set_trace_id( objtext__ );  // unique name for the trace, can be used as 'data storage name'
    desc.set_trace_display_name( L"DC122 Waveforms" );
    desc.set_axis_label( so::Description::axisX, L"Time" );
    desc.set_axis_label( so::Description::axisY, L"mV" );
    desc.set_axis_decimals( so::Description::axisX, 3 );
    desc.set_axis_decimals( so::Description::axisY, 3 );
    desc.set_objid( objid_ );
    desc.set_objtext( objtext__ );
    setDescription( desc );
}

WaveformObserver::~WaveformObserver()
{
}

const char * 
WaveformObserver::objtext() const
{
    return objtext__;
}

const boost::uuids::uuid&
WaveformObserver::objid() const
{
    return objid_;
}

uint64_t 
WaveformObserver::uptime() const 
{
    return 0;
}

void 
WaveformObserver::uptime_range( uint64_t& oldest, uint64_t& newest ) const 
{
    oldest = newest = 0;
    
    std::lock_guard< std::mutex > lock( const_cast<WaveformObserver *>( this )->mutex() );

    if ( !que_.empty() ) {
        oldest = que_.front()->pos();
        newest = que_.back()->pos();
    }
    
}

std::shared_ptr< so::DataReadBuffer >
WaveformObserver::readData( uint32_t pos )
{
    std::lock_guard< std::mutex > lock( mutex() );
    
    if ( que_.empty() )
        return 0;

    if ( pos == std::numeric_limits<uint32_t>::max() ) {
        return que_.back();
    }

    auto it = std::find_if( que_.begin(), que_.end()
                            , [pos]( const std::shared_ptr< so::DataReadBuffer >& p ){ return pos == p->pos(); } );
    if ( it != que_.end() )
        return *it;
    
    return 0;
}

int32_t
WaveformObserver::posFromTime( uint64_t usec ) const 
{
    std::lock_guard< std::mutex > lock( const_cast< WaveformObserver *>(this)->mutex() );
    
    if ( que_.empty() )
        return false;

    auto it = std::lower_bound( que_.begin(), que_.end(), usec
                                , [] ( const std::shared_ptr< so::DataReadBuffer >& p, uint64_t usec ) { return p->timepoint() < usec; } );
    if ( it != que_.end() )
        return (*it)->pos();

    return 0;
}

uint32_t
WaveformObserver::push_back( std::shared_ptr< acqrscontrols::aqdrv4::waveform > d )
{
    auto rb = std::make_shared< so::DataReadBuffer >();

    auto pos = d->serialNumber();
    
    rb->pos() = pos;
    rb->timepoint() = d->timeStamp();

    auto ap240w = std::make_shared< acqrscontrols::ap240::waveform >();
    
    auto it = std::find_if( methods_.begin(), methods_.end()
                            , [&]( const std::pair< uint32_t, std::shared_ptr< const acqrscontrols::ap240::method > >& a ){
                                return a.first == d->methodNumber();
                            });

    if ( it != methods_.end() ) {
        ap240w->method_ = *it->second;
        if ( it != methods_.begin() )
            methods_.erase( methods_.begin(), it - 1 );
    } else {
        ADDEBUG() << "method " << d->methodNumber()
                  << " not found, list top is " << ( methods_.empty() ? 0 : methods_.front().first );
    }

    ap240w->move( std::move( d ) );
    
    rb->setData( boost::any( const_waveform_pair_t( ap240w, 0 ) ) );
    
    std::lock_guard< std::mutex > lock( mutex() );

    if ( que_.size() > 2000 ) { // 2 seconds @ 1kHz
        auto tail = que_.begin();
        std::advance( tail, 500 );
        que_.erase( que_.begin(), tail );
    }
    
    que_.emplace_back( std::move( rb ) );
    
    return pos;
}

void
WaveformObserver::prepare_for_run( uint32_t methodNumber, std::shared_ptr< const acqrscontrols::ap240::method > p )
{
    std::lock_guard< std::mutex > lock( mutex() );    
    methods_.emplace_back( methodNumber, p );
}

