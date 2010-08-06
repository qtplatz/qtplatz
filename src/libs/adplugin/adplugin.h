// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ADPLUGIN_H
#define ADPLUGIN_H

#include "adplugin_global.h"

namespace adplugin {

	static const char * iid_iMonitor =             "adplugin.ui.iMonitor";
	static const char * iid_iControlMethodEditor = "adplugin.ui.iControlMethodEditor";

	class ADPLUGINSHARED_EXPORT manager {
	protected:
		manager();
        ~manager();
	public:
		static manager * instance();
		static void dispose();

		virtual bool loadConfig( const wchar_t * ) = 0;

	private:
        static manager * instance_;
	};
}

//Q_DECLARE_INTERFACE(::adplugin::ui::MonitorInterface, "MonitorInterface/1.0" );
//Q_DECLARE_INTERFACE(::adplugin::ui::MethodInterface, "MethodInterface/1.0" );



#endif // ADPLUGIN_H
