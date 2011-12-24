// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "task.hpp"
#include <boost/format.hpp>
#include <acewrapper/mutex.hpp>
#include <acewrapper/mutex.hpp>
#include "message.hpp"
#include <acewrapper/timeval.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/description.hpp>
//--
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <adutils/processeddata.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <iostream>

#if defined _MSC_VER
# pragma warning(disable:4996)
#endif
# include <ace/Reactor.h>
# include <ace/Thread_Manager.h>
# include <adinterface/signalobserverC.h>
#if defined _MSC_VER
# pragma warning(default:4996)
#endif

using namespace adbroker;

namespace adbroker {
    namespace internal {
 
        struct portfolio_created {
            std::wstring token;
            portfolio_created( const std::wstring& tok ) : token( tok ) {}
            void operator () ( Task::session_data& d ) const {
                d.receiver_->portfolio_created( token.c_str() );
            }
        };
        //--------------------------
        struct folium_added {
            std::wstring token_;
            std::wstring path_;
            std::wstring id_;
            folium_added( const std::wstring& tok, const std::wstring& path, const std::wstring id )
                : token_( tok ), path_(path), id_(id) {
            }
            void operator () ( Task::session_data& d ) const {
                d.receiver_->folium_added( token_.c_str(), path_.c_str(), id_.c_str() );
            }
        };
    }
}

Task::~Task()
{
}

Task::Task( size_t n_threads ) : barrier_(n_threads)
                               , n_threads_(n_threads) 
{
}

int
Task::handle_timer_timeout( const ACE_Time_Value& tv, const void * )
{
    (void)tv;
    return 0;
}

bool
Task::open()
{
    if ( activate( THR_NEW_LWP, n_threads_ ) != - 1 )
        return true;
    return false;
}

void
Task::close()
{
    do {
        // this will block until a message arrives.
        // By blocking, we know that the destruction will be
        // paused until the last thread is done with the message
        // block
        ACE_Message_Block * mblk = new ACE_Message_Block( 0, ACE_Message_Block::MB_HANGUP );
        putq( mblk );
    } while (0);

    this->wait();
    this->msg_queue()->deactivate();
    ACE_Task<ACE_MT_SYNCH>::close( 0 );

    delete this;
}

bool
Task::connect( Broker::Session_ptr session, BrokerEventSink_ptr receiver, const char * token )
{
    session_data data;
    data.token_ = token;
    data.session_ = Broker::Session::_duplicate( session );
    data.receiver_ = BrokerEventSink::_duplicate( receiver );

    acewrapper::scoped_mutex_t<> lock( mutex_ );

    if ( std::find(session_set_.begin(), session_set_.end(), data ) != session_set_.end() )
        return false;
  
    session_set_.push_back( data );

    return true;
}

bool
Task::disconnect( Broker::Session_ptr session, BrokerEventSink_ptr receiver )
{
    session_data data;
    data.session_ = Broker::Session::_duplicate( session );
    data.receiver_ = BrokerEventSink::_duplicate( receiver );

    acewrapper::scoped_mutex_t<> lock( mutex_ );

    vector_type::iterator it = std::remove( session_set_.begin(), session_set_.end(), data );

    if ( it != session_set_.end() ) {
        session_set_.erase( it, session_set_.end() );
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////
bool
Task::session_data::operator == ( const session_data& t ) const
{
    return receiver_->_is_equivalent( t.receiver_.in() )
        && session_->_is_equivalent( t.session_.in() );
}

bool
Task::session_data::operator == ( const BrokerEventSink_ptr t ) const
{
    return receiver_->_is_equivalent( t );
}

bool
Task::session_data::operator == ( const Broker::Session_ptr t ) const
{
    return session_->_is_equivalent( t );
}

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
int
Task::handle_input( ACE_HANDLE )
{
    ACE_Message_Block *mb;
    ACE_Time_Value zero( ACE_Time_Value::zero );
    if ( this->getq(mb, &zero) == -1 ) {
        ACE_ERROR((LM_ERROR, "(%t) %p\n", "dequeue_head"));
    } else {
        ACE_Message_Block::release(mb);
    }
    return 0;
}

int
Task::svc()
{
    std::cerr << "Task::svc() task started on thread :" << ACE_Thread::self() << std::endl;

    barrier_.wait();

    for ( ;; ) {

        ACE_Message_Block * mblk = 0;

        if ( this->getq( mblk ) == (-1) ) {
            if ( errno == ESHUTDOWN )
                ACE_ERROR_RETURN((LM_ERROR, "(%t) adbroker::task queue is deactivated\n"), 0);
            else
                ACE_ERROR_RETURN((LM_ERROR, "(%t) %p\n", "putq"), -1);
        }

        if ( mblk->msg_type() == ACE_Message_Block::MB_HANGUP ) {
            std::cerr << "adbroker::task close on thread :" << ACE_Thread::self() << std::endl;
            this->putq( mblk ); // forward the request to any peer threads
            break;
        }
        doit( mblk );
        ACE_Message_Block::release( mblk );
    }
    return 0;
}

void
do_coaddSpectrum( Task * pTask, const wchar_t * token, SignalObserver::Observer_ptr observer, double x1, double x2 )
{
    SignalObserver::Description_var desc = observer->getDescription();
    CORBA::WString_var clsid = observer->dataInterpreterClsid();

    long pos = observer->posFromTime( boost::uint64_t( x1 * 60 * 1000 * 1000 ) ); // us
    long pos2 = observer->posFromTime( boost::uint64_t( x2 * 60 * 1000 * 1000 ) ); // us

    std::wstring text;
    if ( pos == pos2 )
        text = ( boost::wformat( L"Spectrum @ %.3f min" ) % x1 ).str();
    else
        text = ( boost::wformat( L"Spectrum range (%1$.3f - %2$.3f) min" ) % x1 % x2 ).str();

    adcontrols::MassSpectrum ms;

    SignalObserver::DataReadBuffer_var dbuf;

    if ( observer->readData( pos, dbuf ) ) {

        try {
            const adcontrols::MassSpectrometer& spectrometer = adcontrols::MassSpectrometer::get( clsid.in() );
            const adcontrols::DataInterpreter& dataInterpreter = spectrometer.getDataInterpreter();
            if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA 
                 && desc->spectrometer == SignalObserver::eMassSpectrometer ) {
                size_t idData = 0;
                dataInterpreter.translate( ms, dbuf, spectrometer, idData++ );
            }
        } catch ( std::exception& ex ) {
            std::cerr << ex.what() << std::endl;
            return;
        }
        ++pos;
    }
    ms.addDescription( adcontrols::Description( L"create", text ) );

#if defined DEBUG || defined _DEBUG
    std::wcout << L"\nInfo: adbroker task do_coaddSpectrum text = " << text
               << ", ms.size = " << ms.size()
               << std::endl;
#endif

    pTask->internal_coaddSpectrum( token, ms );
}

void
Task::doit( ACE_Message_Block * mblk )
{
    using namespace adbroker;

    TAO_InputCDR cdr( mblk );

    CORBA::WString_var msg, token;
    cdr >> token;
    cdr >> msg;
    if ( std::wstring( msg ) == L"coaddSpectrum" ) {
        SignalObserver::Observer_ptr observer;
        double x1(0), x2(0);
        cdr >> observer;
        cdr >> x1;
        cdr >> x2;
        if ( CORBA::is_nil( observer ) )
            return;
        do_coaddSpectrum( this, token, observer, x1, x2 );
    }
}

portfolio::Portfolio&
Task::getPortfolio( const std::wstring& token )
{
    std::map< std::wstring, boost::shared_ptr<portfolio::Portfolio> >::iterator it = portfolioVec_.find( token );
    if ( it == portfolioVec_.end() ) {

        portfolioVec_[ token ] = boost::shared_ptr<portfolio::Portfolio>( new portfolio::Portfolio );
        portfolioVec_[ token ]->create_with_fullpath( token );

        BOOST_FOREACH( session_data& d, session_set_ ) {
#if defined DEBUG || defined _DEBUG// && 0
            adportable::debug(__FILE__, __LINE__) << "getPortfolio token=" << token << " created and fire to :" << d.token_;
#endif
            d.receiver_->portfolio_created( token.c_str() );
        }
    }
    return *portfolioVec_[ token ];
}

void
Task::internal_coaddSpectrum( const std::wstring& token, const adcontrols::MassSpectrum& src )
{
    portfolio::Portfolio& portfolio = getPortfolio( token );

    portfolio::Folder folder = portfolio.addFolder( L"MassSpectra" );
    portfolio::Folium folium = folder.addFolium( L"MassSpectrum" );

    //------->
    adcontrols::MassSpectrumPtr ms( new adcontrols::MassSpectrum( src ) );  // profile, deep copy
    static_cast<boost::any&>( folium ) = ms;
    //<-------

    std::wstring id = folium.id();

    BOOST_FOREACH( session_data& d, session_set_ ) {
#if defined DEBUG // && 0
        adportable::debug( __FILE__, __LINE__ ) 
            << "===== internal_coaddSpectrum folium id: " << id 
            << " token=" << token;
        portfolio.save( L"/tmp/internal_coaddSpectrum_portfolio.xml" );
#endif
        d.receiver_->folium_added( token.c_str(), L"path", id.c_str() );
    }
}

portfolio::Folium
Task::findFolium( const std::wstring& token, const std::wstring& id )
{
    portfolio::Portfolio& portfolio = getPortfolio( token );

    std::wstring xml = portfolio.xml();

    return portfolio.findFolium( id );
}
