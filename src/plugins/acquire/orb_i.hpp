// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <QObject>

#if HAVE_CORBA
# include "receiver_i.hpp"
# include <adinterface/brokerC.h>
# include <adinterface/controlserverC.h>
# include <adinterface/receiverC.h>
# include <adinterface/observerevents_i.hpp>
#endif

#include <map>

namespace adcontrols { class MassSpectrometer; }

namespace acquire {

    class AcquirePlugin;

    class orb_i : public QObject {

        Q_OBJECT

    public:
        ControlServer::Session_var session_;
        SignalObserver::Observer_var observer_;

        typedef std::tuple< SignalObserver::Observer_var
                            , SignalObserver::Description_var
                            , std::wstring
                            , bool
                            , std::shared_ptr< adcontrols::MassSpectrometer > > observer_type;

        std::map< unsigned long, observer_type > observerMap_;
        std::unique_ptr< receiver_i > receiver_i_;
        std::unique_ptr< adinterface::ObserverEvents_i > sink_;
        std::map< unsigned long, long > npos_map_;

        void actionConnect();
        void actionDisconnect();
        void actionInitRun();
        void actionRun();
        void actionStop();
        void actionInject();
        void actionSnapshot();
        bool readCalibrations( observer_type& );
        
        void handle_update_data( unsigned long objId, long pos );
        void handle_controller_message( unsigned long /* Receiver::eINSTEVENT */ msg, unsigned long value );

        // observer event handlers
        void handle_observer_config_changed( uint32_t objid, SignalObserver::eConfigStatus );
        void handle_observer_update_data( uint32_t objid, int32_t pos );
        void handle_observer_method_changed( uint32_t objid, int32_t pos );
        void handle_observer_event( uint32_t objid, int32_t pos, int32_t events );
        
        // receiver_i handlers
        //void handle_receiver_message( Receiver::eINSTEVENT, uint32_t ); --> handle_controller_message
        void handle_receiver_log( const ::EventLog::LogMessage& );
        void handle_receiver_shutdown();
        void handle_receiver_debug_print( int32_t, int32_t, std::string );

        void initialize();
        void shutdown();

        static orb_i * instance();
        
        orb_i();
        ~orb_i();
    signals:
        void onUpdateUIData( unsigned long, long );        
        void onObserverConfigChanged( unsigned long, long );
        void onObserverMethodChanged( unsigned long, long );
        void onObserverEvent( unsigned long, long, long );
        // receiver signals
        void onReceiverMessage( unsigned long, unsigned long );

    private:
        class task;
        std::unique_ptr< task > task_;

        class impl;
        std::unique_ptr< impl > impl_;
    };
}

