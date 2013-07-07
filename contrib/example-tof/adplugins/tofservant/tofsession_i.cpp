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

#include "tofSession_i.hpp"
#include "toftask.hpp"
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
    return true;
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
    return 0;
}

CORBA::Boolean 
tofSession_i::initialize (void)
{
    return true;
}

SignalObserver::Observer_ptr
tofSession_i::getObserver( void )
{
	return toftask::instance()->getObserver();
}

CORBA::Boolean 
tofSession_i::shutdown (void)
{
    return true;
}

CORBA::Boolean 
tofSession_i::echo (const char * msg)
{
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
    TOF::ControlMethod m;
	toftask::instance()->getControlMethod( m );

    p->lines.length( 1 );
    p->lines[ 0 ].modelname = CORBA::wstring_dup( L"TOF" );
    p->lines[ 0 ].index = 0;
    p->lines[ 0 ].unitnumber = 0;
    p->lines[ 0 ].isInitialCondition = true;
    p->lines[ 0 ].funcid = 0;
    p->lines[ 0 ].data <<= m;

    return p._retn();
}

CORBA::Boolean 
tofSession_i::prepare_for_run ( const ControlMethod::Method& m )
{
    return true;
}

CORBA::Boolean 
tofSession_i::push_back ( SampleBroker::SampleSequence_ptr s )
{
    return false;
}

CORBA::Boolean 
tofSession_i::event_out ( CORBA::ULong event)
{
    return false;
}

CORBA::Boolean 
tofSession_i::start_run (void)
{
    return true;
}

CORBA::Boolean 
tofSession_i::suspend_run (void)
{
    return true;
}

CORBA::Boolean 
tofSession_i::resume_run (void)
{
    return true;
}

CORBA::Boolean 
tofSession_i::stop_run (void)
{
    return true;
}

void
tofSession_i::debug( const CORBA::WChar * text, const CORBA::WChar * key )
{
}

bool
tofSession_i::setControlMethod( const TOF::ControlMethod& method, const char * hint )
{
    (void)method;
    (void)hint;
    return true;
}
