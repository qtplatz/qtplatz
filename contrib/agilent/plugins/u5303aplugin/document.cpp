/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include <u5303a/digitizer.hpp>
#include <boost/bind.hpp>
#include <string>

using namespace u5303a;

document * document::instance_ = 0;
std::mutex document::mutex_;

namespace u5303a { namespace detail {
        struct remover {
            ~remover() {
                if ( document::instance_ ) {
                    std::lock_guard< std::mutex > lock( document::mutex_ );
                    if ( document::instance_ )
                        delete document::instance_;
                }
            };
            static remover _remover;
        };
    }
}
    
document::document() : digitizer_( new u5303a::digitizer )
{
}

document::~document()
{
    delete digitizer_;
}

document *
document::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new document;
    }
    return instance_;
}

void
document::u5303a_connect()
{
    //digitizer_->connect( [=]( const std::string& method, const std::string& reply ){ reply_handler( method, reply );} );
	digitizer_->connect_reply( boost::bind( &document::reply_handler, this, _1, _2 ) );
	digitizer_->connect_waveform( boost::bind( &document::waveform_handler, this, _1 ) );
	digitizer_->peripheral_initialize();
}

void
document::u5303a_prepare_for_run()
{
    method m;
	digitizer_->peripheral_prepare_for_run( m );    
}

void
document::u5303a_start_run()
{
	digitizer_->peripheral_run();
}

void
document::u5303a_stop()
{
	digitizer_->peripheral_stop();
}

void
document::u5303a_trigger_inject()
{
	digitizer_->peripheral_trigger_inject();
}

void
document::reply_handler( const std::string& method, const std::string& reply )
{
	emit on_reply( QString::fromStdString( method ), QString::fromStdString( reply ) );
    if ( method == "InitialSetup" && reply == "success" )
        emit on_status( 1 );
}

void
document::waveform_handler( const waveform * p )
{
    auto ptr = p->shared_from_this();
    std::lock_guard< std::mutex > lock( mutex_ );
    que_.push_back( ptr );
    while ( que_.size() >= 32 )
        que_.pop_front();
    emit on_waveform_recieved();
}
