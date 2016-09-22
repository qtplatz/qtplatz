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

#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif
#include "waveformobserver.hpp"
#include <ap240/digitizer.hpp>
#include <boost/version.hpp>
#if BOOST_VERSION < 106000
#include <boost/uuid/uuid_io.hpp>
#endif
#include <boost/uuid/uuid_generators.hpp>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <acqrscontrols/ap240/waveform.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <adportable/portable_binary_oarchive.hpp>

#include <algorithm>
#include <sstream>

using namespace ap240controller;
static const char * objtext__ = "1.ap240.ms-cheminfo.com";
    
WaveformObserver::WaveformObserver() : objid_( boost::uuids::name_generator( base_uuid() )( objtext__ ) )
{
    so::Description desc;
    desc.set_trace_method( so::eTRACE_SPECTRA );
    desc.set_spectrometer( so::eMassSpectrometer );
    desc.set_trace_id( objtext__ );  // unique name for the trace, can be used as 'data storage name'
    desc.set_trace_display_name( L"AP240 Waveforms" );
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

    auto it = std::find_if( que_.begin(), que_.end(), [pos]( const std::shared_ptr< so::DataReadBuffer >& p ){ return pos == p->pos(); } );
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
WaveformObserver::operator << ( const_waveform_pair_t& pair )
{
    auto rb = std::make_shared< so::DataReadBuffer >();

    if ( pair.first || pair.second ) {

        auto pos = pair.first ? pair.first->serialnumber_ : pair.second->serialnumber_;
        rb->pos() = pos;
        rb->timepoint() = pair.first ? pair.first->timeSinceEpoch_ : pair.second->timeSinceEpoch_;
        rb->setData( boost::any( pair ) );

        // acqrscontrols::ap240::waveform::serialize( *rb, pair.first, pair.second );
    
        std::lock_guard< std::mutex > lock( mutex() );

        if ( que_.size() > 2000 ) { // 2 seconds @ 1kHz
            auto tail = que_.begin();
            std::advance( tail, 500 );
            que_.erase( que_.begin(), tail );
        }

        que_.emplace_back( std::move( rb ) );

        return pos;
    }
    
    return uint32_t( -1 );
}

