// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adplugin_global.h"
#include "ifactory.h"
#include <string>

class QString;
class QObject;
class QWidget;

namespace adportable {
	class Configuration;
	class Component;
}

namespace adplugin {

    class orbLoader;

	class ADPLUGINSHARED_EXPORT manager {
	protected:
            manager();
            ~manager();
	public:
            static manager * instance();
            static void dispose();
            static std::string ior( const char * name ); // return broker::manager's ior
            static std::string iorBroker();
            
            virtual bool loadConfig( adportable::Configuration&, const std::wstring&, const wchar_t * query ) = 0;

            virtual adplugin::ifactory * loadFactory( const std::wstring& ) = 0;
            virtual bool unloadFactory( const std::wstring& ) = 0;

            virtual orbLoader& orbLoader( const std::wstring& name ) = 0;
			virtual void register_ior( const std::string& name, const std::string& ior ) = 0;
            virtual const char * lookup_ior( const std::string& name ) = 0;

            static QWidget * widget_factory( const adportable::Configuration&, const wchar_t * path, QWidget * parent = 0 );
            
	private:

	};
}
