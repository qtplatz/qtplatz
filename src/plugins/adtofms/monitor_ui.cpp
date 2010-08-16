//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "monitor_ui.h"
#include <QtCore/qplugin.h>
#include <adportable/configuration.h>
#include "ui_form.h"
#include <adplugin/orbmanager.h>
#include <acewrapper/nameservice.h>
#include "../../../tofcontroller/tofcontrollerC.h"

using namespace adtofms;

monitor_ui::monitor_ui(QWidget *parent) : IMonitor(parent)
                                        , ui(new Ui::Form)
{
    ui->setupUi(this);
}

monitor_ui::~monitor_ui()
{
	delete ui;
}

void
monitor_ui::OnCreate( const adportable::Configuration& c )
{
	config_ = c;
}

void
monitor_ui::OnInitialUpdate()
{
	// now, it is safe to access CORBA servant
	if ( adplugin::ORBManager::instance()->init( 0, 0 ) >= 0 ) {
		CORBA::Object_var obj = adplugin::ORBManager::instance()->getObject( L"tofcontroller.manager" );
		if ( ! CORBA::is_nil( obj.in() )  ) {
			TOFInstrument::TofSession_var tof = TOFInstrument::TofSession::_narrow( obj );
			if ( ! CORBA::is_nil( tof.in() ) ) {
				CORBA::WString_var version = tof->tof_software_revision();
				long x = 0;
			}
		}
	}
}

void
monitor_ui::OnUpdate( boost::any& )
{
}

void
monitor_ui::OnUpdate( unsigned long lHint )
{
}

void
monitor_ui::OnFinalClose()
{
}

// Q_EXPORT_PLUGIN( monitor_ui )