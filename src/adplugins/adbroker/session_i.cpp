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

#include "session_i.hpp"
#include "manager_i.hpp"
#include "chemicalformula_i.hpp"
#include "brokermanager.hpp"
#include "task.hpp"
#include <portfolio/folium.hpp>
#include <adcontrols/massspectrum.hpp>
#include <acewrapper/orbservant.hpp>
#include <sstream>

namespace adbroker {

	namespace internal {

        struct event_sink {
            inline bool operator == ( const event_sink& t ) const {
                return sink_->_is_equivalent( t.sink_.in() );
            }
            inline bool operator == ( const BrokerEventSink_ptr t ) const {
                return sink_->_is_equivalent( t );
            }
            BrokerEventSink_var sink_;
			std::string token_;
            std::string user_;
            event_sink() {}
            event_sink( const event_sink& t ) : sink_( t.sink_ )
                                              , token_( t.token_ ) {
			}
        };

    }
}

using namespace adbroker;

session_i::session_i( const wchar_t * token ) : token_(token)
{
}

session_i::~session_i(void)
{
}

bool
session_i::connect( const char * user, const char * pass, const char * token, BrokerEventSink_ptr cb )
{
    ACE_UNUSED_ARG( pass );

	adbroker::Task * pTask = adbroker::BrokerManager::task();
    if ( pTask ) {
        if ( ! CORBA::is_nil( cb ) ) {
            internal::event_sink sink;
            sink.sink_ = BrokerEventSink::_duplicate( cb );
            sink.token_ = token;
            sink.user_ = user;
            event_sink_set_.push_back( sink );
            pTask->connect( this->_this(), cb, token );
        }
        return true;
    }
    return false;
}

bool
session_i::disconnect( BrokerEventSink_ptr cb )
{
    if ( ! CORBA::is_nil( cb ) ) {
        auto it = std::find_if( event_sink_set_.begin(), event_sink_set_.end(), [&cb]( const internal::event_sink& t ){
                return t.sink_->_is_equivalent( cb );
            });
        if ( it != event_sink_set_.end() ) {
            adbroker::BrokerManager::task()->disconnect( this->_this(), it->sink_ );
            event_sink_set_.erase( it );
            return true;
        }
    }
    return false;
}

Broker::ChemicalFormula_ptr
session_i::getChemicalFormula()
{
    PortableServer::POA_var poa = ::adbroker::manager_i::instance()->poa();

    if ( CORBA::is_nil( poa ) )
        return 0;

    ChemicalFormula_i * p = new ChemicalFormula_i();
    if ( p ) {
        CORBA::Object_ptr obj = poa->servant_to_reference( p );
        try {
            Broker::ChemicalFormula_var var = Broker::ChemicalFormula::_narrow( obj );
            return var._retn();
        } catch ( CORBA::Exception& ) {
        }
    }
    return 0;
}

bool
session_i::coaddSpectrum( const CORBA::WChar * token, SignalObserver::Observer_ptr observer, CORBA::Double x1, CORBA::Double x2)
{
    adbroker::Task * pTask = adbroker::BrokerManager::task();
    pTask->io_service().post( std::bind(&Task::handleCoaddSpectrum, pTask, std::wstring(token), observer, x1, x2 ) );
	return true;
}

namespace adbroker {

    struct BrokerFoliumBuffer : public std::streambuf {
        Broker::Folium_var var_;
        size_t count_;
        size_t size_;
        unsigned char * p_;

        BrokerFoliumBuffer() : var_( new Broker::Folium ), count_(0), size_(0), p_(0) {
            resize();
        }

        void resize() { 
            var_->serialized.length( var_->serialized.length() + 1024 * 8 );
            size_ = var_->serialized.length();
            p_ = var_->serialized.get_buffer();
        }

        virtual int_type overflow ( int_type c ) {
            if ( count_ >= size_ )
                resize();
            p_[ count_++ ] = c;
            return c;
        }

        virtual std::streamsize xsputn( const char * s, std::streamsize num ) {
            while ( count_ + num >= size_ )
                resize();
            for ( int i = 0; i < num; ++i )
                p_[ count_++ ] = *s++;
            return num;
        }
    };
}

Broker::Folium *
session_i::folium( const CORBA::WChar * token, const CORBA::WChar * fileId )
{
#if 0
	// since on-memory 'portfolio' for snapshot spectra holder was altered by direct adfs use, 
	// this method may not be necessary.  This will resume when distributed filesystem supported.
    BrokerFoliumBuffer buffer;

    adbroker::Task * pTask = adbroker::BrokerManager::task();
    if ( pTask ) {
        adcontrols::MassSpectrumPtr ptr;
        portfolio::Folium folium = pTask->findFolium( token, fileId );
        if ( portfolio::Folium::get< adcontrols::MassSpectrumPtr >( ptr, folium ) ) {
            std::ostream ostm( &buffer );
            adcontrols::MassSpectrum::archive( ostm, *ptr );
        }
    }
    return buffer.var_._retn();
#endif
	return 0;
}
