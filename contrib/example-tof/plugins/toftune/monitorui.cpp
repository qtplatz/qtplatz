/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "monitorui.hpp"
#include "ui_monitorui.h"

#include <tofinterface/tofC.h>
#include <adinterface/receiverS.h>
#include <adinterface/brokerC.h>
//#include <acewrapper/brokerhelper.hpp>
#include <adplugin/adplugin.hpp>
#include <adorbmgr/orbmgr.hpp>
#include <adinterface/eventlog_helper.hpp>
#include <qtwrapper/qstring.hpp>

namespace toftune {

	class Receiver_i : public POA_Receiver {
		MonitorUI& parent_;
	public:
		~Receiver_i();
		Receiver_i( MonitorUI& );
		TOF::Session_var session_;

		// POA_Receiver
		void message( Receiver::eINSTEVENT msg, CORBA::ULong value );
		void log( const EventLog::LogMessage& );
		void shutdown();
		void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text );
		// <--
		// template<class T> T& get();
	private:
	};
}

using namespace toftune;

MonitorUI::MonitorUI(QWidget *parent) : IMonitor(parent)
                                      , ui(new Ui::MonitorUI)
{
    ui->setupUi(this);

    connect( ui->spinSamplingInterval, SIGNAL( valueChanged( int ) ), this, SLOT( handleSampInterval( int ) ) );
    connect( ui->spinPeakWidth, SIGNAL( valueChanged( int ) ), this, SLOT( handlePeakWidth( int ) ) );

	receiver_i_ = new Receiver_i( *this );
}

MonitorUI::~MonitorUI()
{
	adorbmgr::orbmgr::instance()->deactivate( receiver_i_->_this() );
    delete ui;
	delete receiver_i_;
}

void
MonitorUI::OnCreate( const adportable::Configuration& c )
{
	config_ = c;
}

void
MonitorUI::OnInitialUpdate()
{
	Broker::Manager_var mgr = adorbmgr::orbmgr::getBrokerManager();

	CORBA::Object_var obj = mgr->find_object( "com.ms-cheminfo.qtplatz.instrument.session.tofservant" );
	if ( ! CORBA::is_nil( obj.in() ) ) {
		receiver_i_->session_ = TOF::Session::_narrow( obj );
        if ( ! CORBA::is_nil( receiver_i_->session_.in() ) ) {
			if ( receiver_i_->session_->connect( receiver_i_->_this(), "tofTunePlugin::MonitorUI" ) ) {
                connect( this, SIGNAL( signal_message( unsigned long, unsigned long ) ), this, SLOT( handle_message( unsigned long, unsigned long ) ) );
			}
		}
	}
}

void
MonitorUI::handle_message( unsigned long msg, unsigned long value )
{
	(void)value;
	if ( msg == Receiver::STATE_CHANGED ) {
	} else if ( msg == Receiver::SETPTS_UPDATED ) {
	} else if ( msg == Receiver::ACTUAL_UPDATED ) {
	}
}

void
MonitorUI::handleSampInterval( int value )
{
    std::cout << "handleSampInterval(" << value << ")" << std::endl;
	if ( receiver_i_ && ! CORBA::is_nil( receiver_i_->session_.in() ) ) {
        TOF::ControlMethod m;
        receiver_i_->session_->setControlMethod( m, "sampInterval" );
	}

}

void
MonitorUI::handlePeakWidth( int value )
{
    std::cout << "handlePeakWidth(" << value << ")" << std::endl;
	if ( receiver_i_ && ! CORBA::is_nil( receiver_i_->session_.in() ) ) {
        TOF::ControlMethod m;
        receiver_i_->session_->setControlMethod( m, "peakWidth" );
	}

}


///// lifecycle

void
MonitorUI::OnUpdate( boost::any& )
{
}

void
MonitorUI::OnFinalClose()
{
	// signal/slot disconnect
	disconnect( this, SIGNAL( signal_message( unsigned long, unsigned long ) ), this, SLOT( handle_message( unsigned long, unsigned long ) ) );

	if ( receiver_i_ && ! CORBA::is_nil( receiver_i_->session_.in() ) )
        receiver_i_->session_->disconnect( receiver_i_->_this() );
}

///////////////

Receiver_i::Receiver_i( MonitorUI& t ) : parent_( t )
{
}

Receiver_i::~Receiver_i()
{
}

void
Receiver_i::message( Receiver::eINSTEVENT msg, CORBA::ULong value )
{
	emit parent_.signal_messagee( msg, value );
}

void
Receiver_i::log( const EventLog::LogMessage& log )
{
	std::wstring text = adinterface::EventLog::LogMessageHelper::toString( log );
    if ( parent_.ui ) {
		QString key = qtwrapper::qstring::copy( log.srcId.in() );
        if ( key.isEmpty() )
			return;
		QString qtext = qtwrapper::qstring::copy( text );
        emit parent_.signal_log( qtext, key );
	}
}

void
Receiver_i::shutdown()
{
}

void
Receiver_i::debug_print( CORBA::Long pri, CORBA::Long cat, const char * text )
{
	(void)pri;
	(void)cat;
	(void)text;
}
