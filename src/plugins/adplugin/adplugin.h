// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ADPLUGIN_H
#define ADPLUGIN_H

#include "adplugin_global.h"
#include <QtPlugin>

class ADPLUGINSHARED_EXPORT Adplugin {
public:
	Adplugin();
};

namespace adplugin {

	namespace ui {

		class MonitorInterface {
		public:
			virtual QString name() const = 0;
		};

		class MethodInterface {
		public:
			virtual QString name() const = 0;
		};

	}
}

Q_DECLARE_INTERFACE(adplugin::ui::MonitorInterface, "MonitorInterface/1.0" );
Q_DECLARE_INTERFACE(adplugin::ui::MethodInterface, "MethodInterface/1.0" );

#endif // ADPLUGIN_H
