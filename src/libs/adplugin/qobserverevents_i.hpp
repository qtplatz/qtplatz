// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once

#include "adplugin_global.h"
#include <QObject>
#include <boost/noncopyable.hpp>

#if defined _MSC_VER
# pragma warning(disable:4996)
#endif

#include <adinterface/signalobserverS.h>

#if defined _MSC_VER
# pragma warning(default:4996)
#endif

namespace adplugin {

	class ObserverEvents_i;

    class ADPLUGINSHARED_EXPORT QObserverEvents_i : public QObject
		                                          , boost::noncopyable {
        Q_OBJECT
    public:
        explicit QObserverEvents_i(QObject *parent = 0);
        QObserverEvents_i( SignalObserver::Observer_ptr
                           , const std::wstring& token 
                           , SignalObserver::eUpdateFrequency freq = SignalObserver::Friquent
                           , QObject * parent = 0 );
        ~QObserverEvents_i();
	private:
		friend class ObserverEvents_i;
        // implements ObserverEvents
        void onConfigChanged( CORBA::ULong objId, SignalObserver::eConfigStatus );
        void onUpdateData( CORBA::ULong, CORBA::Long );
        void onMethodChanged( CORBA::ULong, CORBA::Long );
        void onEvent( CORBA::ULong, CORBA::ULong, CORBA::Long );
		// end ObserverEvents
	public:
		void disconnect();
		::SignalObserver::ObserverEvents *_this (void);
	public:
        // Observer 
        inline SignalObserver::Observer_ptr& ptr() { return impl_; }
    signals:
        void signal_OnClose();
        void signal_UpdateData( unsigned long, long );
        void signal_MethodChanged( unsigned long, long );
        void signal_Event( unsigned long, unsigned long, long );
        void signal_ConfigChanged( unsigned long, long );
                                                               
    public slots:
	
    private:
        SignalObserver::Observer * impl_;
		ObserverEvents_i * sink_;
        std::wstring token_;
        SignalObserver::eUpdateFrequency freq_;
        unsigned long objId_;
        bool connected_;
    };
}

