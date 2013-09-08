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

#include "tofsession_i.hpp"
#include <tofinterface/method.hpp>
#include "toftask.hpp"
#include <adportable/serializer.hpp>
#include <boost/tokenizer.hpp>


using namespace tofservant;

//////////////////////////////////
CORBA::Char * 
tofSession_i::software_revision (void)
{
    return CORBA::string_dup("1.0.0.0");
}

CORBA::Boolean 
tofSession_i::setConfiguration( const char * xml )
{
	return toftask::instance()->setConfiguration( xml );
}

CORBA::Boolean 
tofSession_i::configComplete()
{
    return true;
}

CORBA::Boolean 
tofSession_i::connect( Receiver_ptr receiver, const CORBA::Char * token )
{
	return toftask::instance()->connect( receiver, token );
}

CORBA::Boolean 
tofSession_i::disconnect ( Receiver_ptr receiver )
{
	return toftask::instance()->disconnect( receiver );
}

CORBA::ULong 
tofSession_i::get_status (void)
{
    return toftask::instance()->get_status();
}

CORBA::Boolean 
tofSession_i::initialize (void)
{
    return toftask::instance()->initialize();
}

SignalObserver::Observer_ptr
tofSession_i::getObserver( void )
{
	return toftask::instance()->getObserver();
}

CORBA::Boolean 
tofSession_i::shutdown (void)
{
	toftask::instance()->task_close();
	return true;
}

CORBA::Boolean 
tofSession_i::echo (const char * msg)
{
	(void)msg;
    return true;
}

CORBA::Boolean 
tofSession_i::shell (const char * cmdline)
{
    if ( cmdline && *cmdline ) {

        typedef boost::char_separator<char> char_separator;
        typedef boost::tokenizer< char_separator > tokenizer;
    
        char_separator sep(" ", "", boost::drop_empty_tokens );
        std::string line( cmdline );
        tokenizer tokens( line, sep );
        std::vector< std::string > argv;
        for ( tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++it )
            argv.push_back( *it );

        return true;
    }
    return false;
}

ControlMethod::Method *
tofSession_i::getControlMethod()
{
    ControlMethod::Method_var p( new ControlMethod::Method() );
    tof::ControlMethod m;
	toftask::instance()->getControlMethod( m );

    p->lines.length( 1 );
    p->lines[ 0 ].modelname = CORBA::wstring_dup( L"TOF" );
    p->lines[ 0 ].index = 0;
    p->lines[ 0 ].unitnumber = 0;
    p->lines[ 0 ].isInitialCondition = true;
    p->lines[ 0 ].funcid = 0;

	std::string device;
	adportable::serializer< tof::ControlMethod >::serialize( m, device );
	p->lines[ 0 ].xdata.length( device.size() );
	std::copy( device.begin(), device.end(), p->lines[0].xdata.get_buffer() );

    return p._retn();
}

CORBA::Boolean 
tofSession_i::prepare_for_run ( const ControlMethod::Method& m )
{
	toftask * task = toftask::instance();
    task->io_service().post( std::bind(&toftask::handle_prepare_for_run, task, m ) );
    return true;
}

CORBA::Boolean 
tofSession_i::push_back ( SampleBroker::SampleSequence_ptr s )
{
	(void)s;
    return false;
}

CORBA::Boolean 
tofSession_i::event_out ( CORBA::ULong event)
{
	toftask * task = toftask::instance();
    task->io_service().post( std::bind(&toftask::handle_event_out, task, event ) );
    return false;
}

CORBA::Boolean 
tofSession_i::start_run (void)
{
    toftask * task = toftask::instance();
    task->io_service().post( std::bind(&toftask::handle_start_run, task ) );
	return true;
}

CORBA::Boolean 
tofSession_i::suspend_run (void)
{
	toftask * task = toftask::instance();
    task->io_service().post( std::bind(&toftask::handle_suspend_run, task ) );
    return true;
}

CORBA::Boolean 
tofSession_i::resume_run (void)
{
    toftask * task = toftask::instance();
    task->io_service().post( std::bind(&toftask::handle_resume_run, task ) );
	return true;
}

CORBA::Boolean 
tofSession_i::stop_run (void)
{
    toftask * task = toftask::instance();
    task->io_service().post( std::bind(&toftask::handle_stop_run, task ) );
	return true;
}

void
tofSession_i::debug( const CORBA::WChar * text, const CORBA::WChar * key )
{
	(void)text;
	(void)key;
}

bool
tofSession_i::setControlMethod( const TOF::octet_array& oa, const char * hint )
{
	tof::ControlMethod method;
	adportable::serializer< tof::ControlMethod >::deserialize( method 
		, reinterpret_cast< const char * >( oa.get_buffer() )
		, oa.length() );
    return toftask::instance()->setControlMethod( method, hint );
}

