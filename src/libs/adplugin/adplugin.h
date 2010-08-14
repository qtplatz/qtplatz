// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ADPLUGIN_H
#define ADPLUGIN_H

#include "adplugin_global.h"
#include <string>

class QString;
class QObject;
class QWidget;

namespace adportable {
	class Configuration;
	class Component;
}

namespace adplugin {

    static const wchar_t * iid_iMonitor =             L"adplugin::ui::iMonitor";
    static const wchar_t * iid_iControlMethodEditor = L"adplugin::ui::iControlMethodEditor";
    static const wchar_t * iid_iLog     =             L"adplugin::ui::iLog";

    class orbLoader;

	class ADPLUGINSHARED_EXPORT manager {
	protected:
            manager();
            ~manager();
	public:
            static manager * instance();
            static void dispose();
            
            virtual bool loadConfig( adportable::Configuration&, const std::wstring&, const wchar_t * query ) = 0;
            virtual QObject * loadLibrary( const std::wstring& ) = 0;
            virtual bool unloadLibrary( const std::wstring& ) = 0;
            
            static QWidget * widget_factory( const adportable::Configuration&, const wchar_t * path, QWidget * parent = 0 );
            
            virtual orbLoader& orbLoader( const std::wstring& name ) = 0;
            
	private:
        // static manager * instance_;
	};
}

//Q_DECLARE_INTERFACE(::adplugin::ui::MonitorInterface, "MonitorInterface/1.0" );
//Q_DECLARE_INTERFACE(::adplugin::ui::MethodInterface, "MethodInterface/1.0" );



#endif // ADPLUGIN_H
