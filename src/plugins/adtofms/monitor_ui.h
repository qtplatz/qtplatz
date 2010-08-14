// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MONITOR_UI_H
#define MONITOR_UI_H

#include <adplugin/imonitor.h>

namespace Ui {
    class Form;
}

namespace adtofms {

	class monitor_ui : public adplugin::ui::IMonitor	{
		Q_OBJECT
		Q_INTERFACES( adplugin::ui::IMonitor )
	public:
		explicit monitor_ui(QWidget *parent = 0);
        ~monitor_ui();

		// implement IMonitor
        virtual void OnCreate( const adportable::Configuration& );
        virtual void OnInitialUpdate();
        virtual void OnUpdate( boost::any& );
        virtual void OnUpdate( unsigned long );
		virtual void OnFinalClose();

    signals:

	public slots:
	private:
		Ui::Form *ui;
	};

}

#endif // MONITOR_UI_H
