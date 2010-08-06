//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "factory_impl.h"
#include <adplugin/adplugin.h>
#include <QtCore/qplugin.h>
#include "monitor_ui.h"

using namespace adtofms;

factory_impl::factory_impl(QObject *parent) :
    adplugin::IFactory(parent)
{
}

QWidget *
factory_impl::create_widget( const char * iid, QWidget * parent )
{
	if ( std::string( iid ) == adplugin::iid_iMonitor ) {
		return new monitor_ui( parent );
	}
	return 0;
}

QObject *
factory_impl::create_object( const char *, QObject * parent )
{
	return 0;
}

Q_EXPORT_PLUGIN( factory_impl )
