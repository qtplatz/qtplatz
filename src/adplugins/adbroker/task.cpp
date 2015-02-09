// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <acewrapper/input_buffer.hpp>
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
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adportable/float.hpp>
#include <adportable/utf.hpp>
#include <adlog/logger.hpp>
#include <sstream>
#include <iostream>
#include <adinterface/signalobserverC.h>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <thread>

using namespace adbroker;

namespace adbroker {

    namespace internal {
 
        struct portfolio_created {
            std::wstring token;
            std::string u8token;
            portfolio_created( const std::wstring& tok ) : token( tok ), u8token( adportable::utf::to_utf8( tok ) ) {}
            void operator () ( Task::session_data& d ) const {
                d.receiver_->portfolio_created( u8token.c_str() );
            }
        };
        //--------------------------
        struct folium_added {
            std::string token_;
            std::string path_;
            std::string id_;
            folium_added( const std::wstring& tok, const std::wstring& path, const std::wstring id )
                : token_( adportable::utf::to_utf8( tok ) ), path_( adportable::utf::to_utf8( path ) ), id_( adportable::utf::to_utf8( id ) ) {
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
    threads_.push_back( adportable::asio::thread( boost::bind(&boost::asio::io_service::run, &io_service_) ) );

    return true;
}

int
Task::task_close()
{
    io_service_.stop();

    for ( auto& t: threads_ )
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
    CORBA::String_var clsid = observer->dataInterpreterClsid();

    if ( ! ( ( desc->trace_method == SignalObserver::eTRACE_SPECTRA ) &&
             ( desc->spectrometer == SignalObserver::eMassSpectrometer ) ) )
        return;

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

        ADTRACE() << "broker readData( " << pos << " ) fnc= " << dbuf->fcn;
        while ( dbuf->fcn != 0 ) {
            if ( !observer->readData( --pos, dbuf ) )
                return;
            ADTRACE() << "back tracking readData( " << pos << " ) fnc= " << dbuf->fcn;
        }

        adcontrols::translate_state state;
        do {
            try {
                size_t idData = 0;
                state = dataInterpreter.translate( ms
                                                      , reinterpret_cast< const char *>( dbuf->xdata.get_buffer() ), dbuf->xdata.length()
                                                      , reinterpret_cast< const char *>( dbuf->xmeta.get_buffer() ), dbuf->xmeta.length()
                                                      , spectrometer, idData++, 0 );
            } catch ( std::exception& ex ) {
                std::cerr << ex.what() << std::endl;
                return;
            }

            bool success( false );
            size_t wait = 3;
            std::chrono::milliseconds duration( 1000 );
            ++pos;
            do {
                if ( ! ( success = observer->readData( pos, dbuf ) ) ) {
                    std::this_thread::sleep_for( duration );
                    ADTRACE() << "waiting for spectrum functions to be completed: " << pos;
                }
            } while ( !success && wait-- );

            if ( ! success )
                return;

        } while ( state == adcontrols::translate_indeterminate );
	}
    // get here even readData or dataInterpreter fails, anyway send data to client
    ms.addDescription( adcontrols::description( L"create", text ) );
	appendOnFile( token, ms, text );
}

void
Task::appendOnFile( const std::wstring& filename, const adcontrols::MassSpectrum& ms, const std::wstring& title )
{
	boost::filesystem::path path( filename );

    adfs::filesystem fs;
	
	if ( ! boost::filesystem::exists( path ) ) {
		if ( ! fs.create( path.wstring().c_str() ) )
			return;
	} else {
		if ( ! fs.mount( path.wstring().c_str() ) )
			return;
	}
	adfs::folder folder = fs.addFolder( L"/Processed/Spectra" );

    std::wstring id;
    if ( folder ) {
		adfs::file file = folder.addFile( adfs::create_uuid(), title );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            id = file.id();
            if ( file.save( ms ) )
				file.commit();
        }
	}

    for ( session_data& d: session_set_ )
        d.receiver_->folium_added( adportable::utf::to_utf8( filename ).c_str(), "/Processed/Spectra", adportable::utf::to_utf8( id ).c_str() );
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
