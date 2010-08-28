// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adplugin_global.h"
#include <QObject>
#include <adinterface/signalobserverS.h>

namespace adplugin {

	class ADPLUGINSHARED_EXPORT QObserverEvents_i : public QObject
		                                          , public POA_SignalObserver::ObserverEvents {
		Q_OBJECT
	public:
		explicit QObserverEvents_i(QObject *parent = 0);
       
        // implements ObserverEvents
		void OnUpdateData( CORBA::Long );
		void OnMethodChanged( CORBA::Long );
		void OnEvent( CORBA::ULong, CORBA::Long );

    signals:
        void signal_UpdateData( unsigned long );
        void signal_MethodChanged( unsigned long );
        void signal_Event( unsigned long, long );

	public slots:

    };
}

