/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#include "toftask.hpp"
#include "tofservant.hpp"
#include "tofmgr_i.hpp"
#include "profileobserver_i.hpp"
#include "traceobserver_i.hpp"
#include <adinterface/signalobserverC.h>
#include <boost/format.hpp>

namespace tofservant {

    struct observer_events_data {
        bool operator == ( const observer_events_data& ) const;
        bool operator == ( const SignalObserver::ObserverEvents_ptr ) const;
        ~observer_events_data() {    }
        observer_events_data();
        observer_events_data( const observer_events_data& );
        SignalObserver::ObserverEvents_var cb_;
        SignalObserver::eUpdateFrequency freq_;
        std::string token_;
        bool failed_;
    };
    
    struct receiver_data {
        bool operator == ( const receiver_data& ) const;
        bool operator == ( const Receiver_ptr ) const;
        ~receiver_data() { /**/ }
        receiver_data() : failed_( false ) { /**/ }
        receiver_data( const receiver_data& t ) : receiver_(t.receiver_)
                                                , failed_( t.failed_ ), token_( t.token_ ) { /**/ }
        Receiver_var receiver_;
        bool failed_;
        std::string token_;
    };

}

using namespace tofservant;

toftask * toftask::instance_ = 0;
std::mutex toftask::mutex_;

toftask::toftask()
{
}

toftask::~toftask()
{
}

toftask *
toftask::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new toftask;
    }
    return instance_;
}

SignalObserver::Observer *
toftask::getObserver()
{
	PortableServer::POA_var poa = tofServantPlugin::instance()->poa();

    if ( ! pObserver_ ) {
        std::lock_guard< std::mutex > lock( mutex_ );

        if ( ! pObserver_ )
            pObserver_.reset( new profileObserver_i );

        // add Traces
        SignalObserver::Description desc;
        desc.trace_method = SignalObserver::eTRACE_TRACE;
        desc.spectrometer = SignalObserver::eMassSpectrometer;
        desc.axis_x_decimals = 3;
        desc.axis_y_decimals = 2;
        desc.trace_display_name = CORBA::wstring_dup( L"TIC" );
        desc.trace_id = CORBA::wstring_dup( L"MS.TIC" );
        desc.trace_method = SignalObserver::eTRACE_TRACE;
        
        std::shared_ptr< traceObserver_i > p( new traceObserver_i );
        p->setDescription( desc );
        pTraceObserverVec_.push_back( p );
        do {
            CORBA::Object_var obj = poa->servant_to_reference( p.get() );
            pObserver_->addSibling( SignalObserver::Observer::_narrow( obj ) );
        } while(0);
        
        // add mass chromatograms
        for ( int i = 0; i < 3; ++i ) {
            std::shared_ptr< traceObserver_i > p( new traceObserver_i );
            desc.trace_method = SignalObserver::eTRACE_TRACE;
            desc.spectrometer = SignalObserver::eMassSpectrometer;
            desc.trace_display_name 
                = CORBA::wstring_dup( ( boost::wformat( L"TOF Chromatogram.%1%" ) % (i + 1) ).str().c_str() );
            desc.trace_id 
                = CORBA::wstring_dup( (boost::wformat( L"MS.CHROMATOGRAM.%1%" ) % (i + 1) ).str().c_str() );
            p->setDescription( desc );
            pTraceObserverVec_.push_back( p );
            do {
                CORBA::Object_var obj = poa->servant_to_reference( p.get() );
                pObserver_->addSibling( SignalObserver::Observer::_narrow( obj ) );
            } while(0);
        }
    }

    CORBA::Object_var obj = poa->servant_to_reference( pObserver_.get() );
    return SignalObserver::Observer::_narrow( obj._retn() );
}

bool
toftask::getControlMethod( TOF::ControlMethod& m )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    m = method_;
    return true;
}

bool
toftask::putq( ACE_Message_Block * )
{
    return true;
}

bool
toftask::connect( Receiver_ptr, const std::string& )
{
    return true;
}

bool
toftask::disconnect( Receiver_ptr )
{
    return true;
}

bool
toftask::connect ( SignalObserver::ObserverEvents_ptr
                   , SignalObserver::eUpdateFrequency, const std::string& )
{
    return true;
}

bool
toftask::disconnect( SignalObserver::ObserverEvents_ptr )
{
    return true;
}

void
toftask::observer_fire_on_update_data( unsigned long objId, long pos )
{
}

void
toftask::observer_fire_on_method_changed( unsigned long objId, long pos )
{
}

void
toftask::observer_fire_on_event( unsigned long objId, unsigned long event, long pos )
{
}

void
toftask::session_fire_message( Receiver::eINSTEVENT msg, unsigned long value )
{
}

void
toftask::session_fire_log( long pri, const std::wstring& format, const std::vector< std::wstring >& args
                           , const std::wstring& msgId )
{
}

