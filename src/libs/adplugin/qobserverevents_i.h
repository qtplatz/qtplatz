// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adplugin_global.h"
#include <QObject>
#pragma warning(disable:4996)
#include <adinterface/signalobserverS.h>
#pragma warning(default:4996)

namespace adplugin {

	class ADPLUGINSHARED_EXPORT QObserverEvents_i : public QObject
		                                          , public POA_SignalObserver::ObserverEvents {
		Q_OBJECT
	public:
		explicit QObserverEvents_i(QObject *parent = 0);
		QObserverEvents_i( SignalObserver::Observer_ptr
			             , const std::wstring& token 
			             , SignalObserver::eUpdateFrequency freq = SignalObserver::Friquent
						 , QObject * parent = 0 );
       
        // implements ObserverEvents
		void OnUpdateData( CORBA::Long );
		void OnMethodChanged( CORBA::Long );
		void OnEvent( CORBA::ULong, CORBA::Long );
        void OnClose();

        // Observer 
		inline SignalObserver::Observer_ptr& ptr() { return impl_; }
    signals:
        void signal_UpdateData( unsigned long, long );
        void signal_MethodChanged( unsigned long, long );
		void signal_Event( unsigned long, unsigned long, long );

	public slots:

	private:
		unsigned long objId_;
		SignalObserver::eUpdateFrequency freq_;
		std::wstring token_;
		SignalObserver::Observer_var impl_;
    };
}

