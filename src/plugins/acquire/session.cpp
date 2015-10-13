/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
**
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

#include "session.hpp"
//#include "sampleprocessor.hpp"
#include "task.hpp"
#include <adcontrols/samplerun.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <boost/tokenizer.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <iostream>

using namespace acquire;

////////////////////////////////////////////

session::~session()
{
}

session::session()
{
}

std::string
session::software_revision() const
{
    return std::string("3.14");
}

bool
session::connect( adicontroller::Receiver * receiver, const std::string& token )
{
//    if ( ! iTask::instance()->connect( _this(), receiver, token ) ) {
//        throw ControlServer::Session::CannotAdd( "receiver already exist" );
//        return false;
//    }
    return true;
}

bool
session::disconnect( adicontroller::Receiver *receiver )
{
    //return iTask::instance()->disconnect( _this(), receiver );
    return false;
}

bool
session::setConfiguration( const std::string& xml )
{
    return false; // return iTask::instance()->setConfiguration( xml );
}

bool
session::configComplete()
{
    return false; // return iTask::instance()->configComplete();
}

bool
session::initialize()
{
    return false; // return iTask::instance()->initialize();
}

bool
session::shutdown()
{
    return false;// iTask::instance()->close();
    return true;
}

uint32_t // constadicontroller::Instrument::eInstStatus
session::get_status()
{
    return adicontroller::Instrument::eNothing; // iTask::instance()->getStatusCurrent();
}

bool
session::echo( const std::string& msg )
{
    // iTask::instance()->io_service().post( std::bind( &iTask::handle_echo, iTask::instance(), std::string( msg ) ) );
    return true;
}

bool
session::shell( const std::string& cmdline )
{
	(void)cmdline;
    return false;
}

std::shared_ptr< const adcontrols::ControlMethod::Method >
session::getControlMethod()
{
    return 0;
}

//---------
bool
session::prepare_for_run( std::shared_ptr< const adcontrols::ControlMethod::Method > m )
{
#if 0
    auto sr = std::make_shared< adcontrols::SampleRun >();
    auto cm = std::make_shared< adcontrols::ControlMethod::Method >();

    std::wstring xml( adportable::utf::to_wstring( sampleXml ) );
    std::wistringstream is( xml );
    adcontrols::SampleRun::xml_restore( is, *sr );

    adinterface::ControlMethodHelper::copy( *cm, m );

    for ( auto& item : *cm )
        ADDEBUG() << item.modelname() << ", " << item.itemLabel() << ", init: " << item.isInitialCondition() << " time:" << item.time();
    
    iTask::instance()->io_service().post( std::bind(&iTask::handle_prepare_for_run, iTask::instance(), cm, sr ) );
#endif
    return true;
}

bool
session::start_run()
{
    //iTask::instance()->io_service().post( std::bind( &iTask::handle_start_run, iTask::instance() ) );
    return true;
}

bool
session::suspend_run()
{
    return false;
}

bool
session::resume_run()
{
    return false;
}

bool
session::stop_run()
{
    //iTask::instance()->io_service().post( std::bind( &iTask::handle_stop_run, iTask::instance() ) );
    return true;
}

bool
session::event_out( uint32_t value )
{
    //iTask::instance()->io_service().post( std::bind( &iTask::handle_event_out, iTask::instance(), value ) );
    return true;
}

//bool
//session::push_back( SampleBroker::SampleSequence_ptr sequence )
//{
//    return false;
//}

adicontroller::SignalObserver::Observer *
session::getObserver (void)
{
    return task::instance()->masterObserver();
}

#if 0
std::string
session::running_sample()
{
    if ( auto sp = iTask::instance()->getCurrentSampleProcessor() ) {
        if ( auto run = sp->sampleRun() ) {
            std::wostringstream os;
            adcontrols::SampleRun::xml_archive( os, *run );
            std::string utf8 = adportable::utf::to_utf8( os.str().c_str() );
            return CORBA::string_dup( utf8.c_str() );
        }
    }
    return 0;
}
#endif
