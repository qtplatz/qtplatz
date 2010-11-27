//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "factory_impl.h"
#include <adplugin/adplugin.h>
#include <QtCore/qplugin.h>
#include "monitor_ui.h"
#include "debug_ui.h"

using namespace adtofms;

factory_impl::factory_impl()
{
}

QWidget *
factory_impl::create_widget( const wchar_t * iid, QWidget * parent )
{
	if ( std::wstring( iid ) == adplugin::iid_iMonitor ) {
		return new monitor_ui( parent );
	}
    if ( std::wstring( iid ) == L"adplugin::ui::iMonitor2" ) {
		return new debug_ui( parent );
	}
	return 0;
}

QObject *
factory_impl::create_object( const wchar_t *, QObject * parent )
{
    Q_UNUSED( parent );
	return 0;
}

void
factory_impl::release()
{
    delete this;
}

//Q_EXPORT_PLUGIN( factory_impl )
