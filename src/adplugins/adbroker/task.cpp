// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include <acewrapper/input_buffer.hpp>
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

# include <ace/Reactor.h>
# include <ace/Thread_Manager.h>
# include <adinterface/signalobserverC.h>
#include <boost/bind.hpp>

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

Task::Task() : work_( io_service_ )
							   , timer_( io_service_ )
                               , interval_( 1000 ) // ms
{
}

int
Task::task_open()
{
    timer_.cancel();
    initiate_timer();
    threads_.push_back( std::thread( boost::bind(&boost::asio::io_service::run, &io_service_) ) );

    return true;
}

int
Task::task_close()
{
    io_service_.stop();
    for ( std::thread& t: threads_ )
        t.join();

    return 0;
}

bool
Task::connect( Broker::Session_ptr session, BrokerEventSink_ptr receiver, const char * token )
{
    session_data data;
    data.token_ = token;
    data.session_ = Broker::Session::_duplicate( session );
    data.receiver_ = BrokerEventSink::_duplicate( receiver );

    std::lock_guard< std::mutex > lock( mutex_ );

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

    std::lock_guard< std::mutex > lock( mutex_ );

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

void
Task::handleCoaddSpectrum( const std::wstring& token, SignalObserver::Observer_ptr observer, double x1, double x2 )
{
    SignalObserver::Description_var desc = observer->getDescription();
    CORBA::WString_var clsid = observer->dataInterpreterClsid();

    const adcontrols::MassSpectrometer& spectrometer = adcontrols::MassSpectrometer::get( clsid.in() );
    const adcontrols::DataInterpreter& dataInterpreter = spectrometer.getDataInterpreter();

    long pos = observer->posFromTime( boost::uint64_t( x1 * 60 * 1000 * 1000 ) ); // us
    long pos2 = observer->posFromTime( boost::uint64_t( x2 * 60 * 1000 * 1000 ) ); // us

    std::wstring text;
    if ( pos == pos2 )
        text = ( boost::wformat( L" Spectrum @ %.3f min" ) % x1 ).str();
    else
        text = ( boost::wformat( L" Spectrum range (%1$.3f - %2$.3f) min" ) % x1 % x2 ).str();

    adcontrols::MassSpectrum ms;

    SignalObserver::DataReadBuffer_var dbuf;

    if ( observer->readData( pos, dbuf ) ) {
		try {
			if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA 
				&& desc->spectrometer == SignalObserver::eMassSpectrometer ) {
					size_t idData = 0;
					if ( ! dataInterpreter.translate( ms, dbuf, spectrometer, idData++ ) ) { // <-- acquire
						//------- call from dataproc -----
						if ( std::wstring( clsid.in() ) == L"adcontrols::MassSpectrum" ) {
							acewrapper::input_buffer ibuffer( reinterpret_cast< unsigned char * >(dbuf->array.get_buffer())
								, dbuf->array.length() * sizeof( CORBA::Long )  );
							std::istream in( &ibuffer );
							adcontrols::MassSpectrum::restore( in, ms );
						}
						//------------
					}
			}
		} catch ( std::exception& ex ) {
			std::cerr << ex.what() << std::endl;
			return;
		}
		++pos;
	}
    ms.addDescription( adcontrols::Description( L"create", text ) );
    internal_coaddSpectrum( token, ms );
}

portfolio::Portfolio&
Task::getPortfolio( const std::wstring& token )
{
    std::map< std::wstring, std::shared_ptr<portfolio::Portfolio> >::iterator it = portfolioVec_.find( token );
    if ( it == portfolioVec_.end() ) {

        portfolioVec_[ token ] = std::shared_ptr<portfolio::Portfolio>( new portfolio::Portfolio );
        portfolioVec_[ token ]->create_with_fullpath( token );

        for ( session_data& d: session_set_ ) {
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
	portfolio::Folium folium = folder.addFolium( adcontrols::MassSpectrum::dataClass() );

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

void
Task::initiate_timer()
{
    timer_.expires_from_now( boost::posix_time::milliseconds( interval_ ) );
    timer_.async_wait( boost::bind( &Task::handle_timeout, this, boost::asio::placeholders::error ) );
}

void
Task::handle_timeout( const boost::system::error_code& )
{
    initiate_timer();
}
