// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef IMONITOR_H
#define IMONITOR_H

#include "adplugin_global.h"
#include <QWidget>

namespace adplugin {

    class IController;

	namespace ui {
		class ADPLUGINSHARED_EXPORT IMonitor : public QWidget	{
			Q_OBJECT
		public:
			explicit IMonitor(QWidget *parent = 0) : QWidget( parent ) {}
			// 
			virtual void OnInitialUpdate( const wchar_t * xml ) = 0;
			virtual void OnFinalClose() = 0;

        signals:

		public slots:

		};
	}
}

Q_DECLARE_INTERFACE( adplugin::ui::IMonitor, "org.adplugin.ui.imonitor/1.0" );

#endif // IMONITOR_H
