// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ADPLUGIN_H
#define ADPLUGIN_H

#include "adplugin_global.h"

class QString;

namespace adportable {
	class Configuration;
	class Component;
}

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

		virtual bool loadConfig( const QString&, const wchar_t * query ) = 0;
		virtual const adportable::Configuration * getConfiguration( const wchar_t * name ) = 0;
		virtual const adportable::Component * findComponent( const wchar_t * name ) = 0;

	private:
        static manager * instance_;
	};
}

//Q_DECLARE_INTERFACE(::adplugin::ui::MonitorInterface, "MonitorInterface/1.0" );
//Q_DECLARE_INTERFACE(::adplugin::ui::MethodInterface, "MethodInterface/1.0" );



#endif // ADPLUGIN_H
