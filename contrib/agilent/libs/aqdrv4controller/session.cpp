/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "session.hpp"
#include "task.hpp"
#include "waveformobserver.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adacquire/masterobserver.hpp>
#include <adacquire/receiver.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/utf.hpp>
#include <adportable/debug.hpp>
#include <adportable/semaphore.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <sstream>

namespace aqdrv4controller { namespace Instrument {
#if 0
        struct Session::impl {
            
            impl() : work_( io_service_ )
                   , masterObserver_( std::make_shared< adacquire::MasterObserver >( "master.aqdrv4.ms-cheminfo.com" ) )
                   , waveformObserver_( std::make_shared< WaveformObserver >() ) {

                // {5df0f451-4b42-597f-b223-4378f92baa48}
                masterObserver_->addSibling( waveformObserver_.get() );

            }
            
            static std::once_flag flag_, flag2_, flag3_;
            static std::shared_ptr< Session > instance_;
            static std::mutex mutex_;

            boost::asio::io_service io_service_;
            boost::asio::io_service::work work_;
            std::vector< std::thread > threads_;

            typedef std::pair< std::shared_ptr< adi::Receiver >, std::string > client_pair_t;
            std::vector< client_pair_t > clients_;
            inline std::mutex& mutex() { return mutex_; }

            std::shared_ptr< ap240::digitizer > digitizer_;
            std::shared_ptr< adacquire::MasterObserver > masterObserver_;
            std::shared_ptr< WaveformObserver > waveformObserver_;
            
            void reply_message( adi::Receiver::eINSTEVENT msg, uint32_t value ) {
                std::lock_guard< std::mutex > lock( mutex_ );
                for ( auto& r: clients_ )
                    r.first->message( msg, value );
            }
            
            void reply_handler( const std::string& method, const std::string& reply ) {
                if ( method == "InitialSetup" ) {
                    reply_message( adi::Receiver::STATE_CHANGED, ( reply == "success" ) ? adi::Instrument::eStandBy : adi::Instrument::eOff );
                } else {
                    adacquire::EventLog::LogMessage mlog( "%1%", method, "AP240" );
                    mlog << reply;
                    for ( auto& r: clients_ )
                        r.first->log( mlog );
                }
            }
            
            bool waveform_handler( const acqrscontrols::ap240::waveform * ch1, const acqrscontrols::ap240::waveform * ch2, acqrscontrols::ap240::method& next ) {
                if ( masterObserver_ && waveformObserver_ ) {
                    if ( ch1 || ch2 ) {
                        auto pair = std::make_pair( ( ch1 ? ch1->shared_from_this() : 0 ), ( ch2 ? ch2->shared_from_this() : 0 ) );
                        auto pos = (*waveformObserver_) << pair;
                        masterObserver_->dataChanged( waveformObserver_.get(), pos );
                        return false; // no next method changed.
                    }
                }
                return false;
            }
        };

        std::once_flag Session::impl::flag_;
        std::once_flag Session::impl::flag2_;
        std::once_flag Session::impl::flag3_;
        std::mutex Session::impl::mutex_;
        std::shared_ptr< Session > Session::impl::instance_;
#endif        
    }
}

using namespace aqdrv4controller::Instrument;

Session *
Session::instance()
{
    static std::shared_ptr< Session > __instance;
    static std::once_flag flag;

    std::call_once( flag, [&] () { __instance = std::make_shared< Session >(); } );
    return __instance.get();
}

Session::Session()
{
}

Session::~Session()
{
}

std::string
Session::software_revision() const
{
    return "3.3";
}

bool
Session::setConfiguration( const std::string& xml )
{
    using boost::property_tree::ptree;
    
    std::istringstream ss( xml );
    ptree pt;
    boost::property_tree::read_xml( ss, pt );

    if ( pt.get< bool >( "Digitizer.RemoteAccess" ) ) {
        if ( boost::optional< std::string > server = pt.get_optional< std::string >( "Digitizer.RemoteHost" ) )
            remote_server_ = server.get();

        if ( boost::optional< std::string > port = pt.get_optional< std::string >( "Digitizer.Port" ) )
            remote_port_ = port.get();

        if ( remote_port_.empty() )
            remote_port_ = "8010";
    }
    
    return true;
}

bool
Session::configComplete()
{
    return true;    
}
            
bool
Session::disconnect( adacquire::Receiver * receiver )
{
    auto self( receiver->shared_from_this() );

    task::instance()->disconnect_client( self );

    return true;
}
      
uint32_t
Session::get_status()
{
    return 0;
}

adacquire::SignalObserver::Observer *
Session::getObserver()
{
    return task::instance()->masterObserver();
}

bool
Session::connect( adacquire::Receiver * receiver, const std::string& token )
{
    auto ptr( receiver->shared_from_this() );
    
    task::instance()->connect_client( ptr, token );

    return true;
}

bool
Session::initialize()
{
    if ( !remote_server_.empty() ) {

        static std::once_flag flag;

        std::call_once( flag, [&](){
                task::instance()->connect( remote_server_, remote_port_ );
            });
    }
    return true;
}

bool
Session::shutdown()
{
    return task::instance()->finalize();
}

bool
Session::echo( const std::string& msg )
{
    return false;
}

bool
Session::shell( const std::string& cmdline )
{
    return false;    
}

std::shared_ptr< const adcontrols::ControlMethod::Method >
Session::getControlMethod()
{
    return 0;
}

bool
Session::prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m )
{
    if ( m ) {
        auto it = m->find( m->begin(), m->end(), acqrscontrols::ap240::method::clsid() ); // "ap240" );
        if ( it != m->end() ) {
            auto method = std::make_shared< acqrscontrols::ap240::method >();
            if ( it->get<>( *it, *method ) ) {
                
                task::instance()->prepare_for_run( method );

                return true;
            }
        }
    }
    return false;
}

bool
Session::time_event_trigger( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                             , adcontrols::ControlMethod::const_time_event_iterator begin
                             , adcontrols::ControlMethod::const_time_event_iterator end )
{
    ADDEBUG() << "TODO -- time_event_triger";
    return true;
}

bool
Session::event_out( uint32_t event )
{
    // if ( event == adacquire::Instrument::instEventInjectOut )
    task::instance()->event_out( event ); // trigger_inject();
    return true;
}

bool
Session::start_run()
{
    return true; //impl_->digitizer_->peripheral_run();
}

bool
Session::suspend_run()
{
    return true; // digitizer has no timed event.
}

bool
Session::resume_run()
{
    return true;  // digitizer has no timed event.
}

bool
Session::stop_run()
{
    return true; // impl_->digitizer_->peripheral_stop();
}

bool
Session::recording( bool enable )
{
    // if ( auto digi = impl_->digitizer_ ) {
    //     if ( enable )
    //         digi->peripheral_prepare_for_run();
    //     else
    //         digi->peripheral_stop();
    // }
    return true;
}

bool
Session::isRecording() const
{
    return true;
}
