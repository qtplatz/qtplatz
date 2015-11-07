// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#ifndef ACQUIREPLUGIN_H
#define ACQUIREPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <vector>
#include <deque>
#include <map>

#if defined HAVE_CORBA
#include <adinterface/controlserverC.h>
#include <adinterface/signalobserverC.h>
#include <adinterface/receiverC.h>
#include <adplugin/orbservant.hpp>
#endif

#if ! defined Q_MOC_RUN
#include <workaround/boost/asio.hpp>
#endif
#include <adportable/asio/thread.hpp>
#include <thread>
#include <mutex>


class QToolButton;
class QAction;
class QComboBox;
class QPointF;
class QRectF;

namespace Core { class IMode; }

namespace adcontrols {
    class MassSpectrometer;
    class DataInterpreter;
    class MassSpectrum;
    class TraceAccessor;
	class MSCalibrateResult;
}

namespace adportable {
    class Configuration;
}

namespace adinterface {
    class ObserverEvents_i;
}

namespace EventLog {
    struct LogMessage;
}

namespace acquire {

    class MainWindow;
    class AcquireImpl;
#if HAVE_CORBA
    class receiver_i;
    class brokerevent_i;
    class ObserverEvents_i;
    class orb_i;
#endif

    //------------
    class AcquirePlugin : public ExtensionSystem::IPlugin {

        Q_OBJECT
        Q_PLUGIN_METADATA(IID "com.ms-cheminfo.qtplatz.plugin" FILE "acquire.json")

    public:
        ~AcquirePlugin();
        AcquirePlugin();

        // implement IPlugin
        bool initialize(const QStringList &arguments, QString *error_message) override;
        void extensionsInitialized() override;
        ShutdownFlag aboutToShutdown() override;

    public:
        static QToolButton * toolButton( QAction * action );
                                              
    public slots:
        void handleCommitMethods();

    private slots:
        void actionConnect();
        void actionDisconnect();
        void actionSnapshot();
        void actionInitRun();
        void actionRun();
        void actionStop();
        void actionInject();
        void actMethodOpen();
        void actMethodSave();

        void handle_shutdown();
        void handle_debug_print( unsigned long priority, unsigned long category, QString text );

        void handle_config_changed( unsigned long objid, long pos );
        void handle_method_changed( unsigned long objid, long pos );
        void handle_event( unsigned long objid, long pos, long flags );
        void handle_update_ui_data( unsigned long objid, long pos );

        void handle_monitor_selected( int );
        void handle_monitor_activated( int );

        void handleSelected( const QPointF& );
        void handleSelected( const QRectF& );

        void handle_broker_initialized();

        void handleReceiverMessage( unsigned long, unsigned long );

    signals:
        // observer signals
        void onUpdateUIData( unsigned long, long );
        void onObserverConfigChanged( unsigned long, long );
        void onObserverMethodChanged( unsigned long, long );
        void onObserverEvent( unsigned long, long, long );

        // receiver signals
        void onReceiverMessage( unsigned long, unsigned long );

        // 
    private:
        void selectPoint( double x, double y );
        void selectRange( double x1, double x2, double y1, double y2 );
        void handle_update_data( unsigned long objid, long pos );
        friend class orb_i;
        orb_i * orb_i_;

        MainWindow * mainWindow_;
        AcquireImpl * pImpl_;

        QAction * actionConnect_;
        QAction * actionRun_;
        QAction * actionInitRun_;
        QAction * actionStop_;
        QAction * actionSnapshot_;
        QAction * actionInject_;

        QAction * actMethodOpen_;
        QAction * actMethodSave_;
        adportable::Configuration * pConfig_;

        void initialize_actions();
        QWidget * createContents( Core::IMode * );
        void initialize_broker();
        void shutdown_broker();

        
        std::map< unsigned long, std::shared_ptr< adcontrols::MassSpectrum > > rdmap_;
        std::deque< std::shared_ptr< adcontrols::MassSpectrum > > fifo_ms_;
        std::map< unsigned long, std::shared_ptr< adcontrols::TraceAccessor > > trace_accessors_;
        std::map< unsigned long, std::shared_ptr< adcontrols::MSCalibrateResult > > calibResults_;

        std::vector< std::wstring > trace_descriptions_;
        QComboBox * traceBox_;

        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        boost::asio::io_service::strand strand_;
        std::vector< adportable::asio::thread > threads_;
        std::mutex mutex_;

        void populate( SignalObserver::Observer_var& );

        bool readMassSpectra( const SignalObserver::DataReadBuffer&
                              , const adcontrols::MassSpectrometer&
                              , const adcontrols::DataInterpreter& dataInterpreter
                              , unsigned long objId );
        bool readTrace( const SignalObserver::Description&
                        , const SignalObserver::DataReadBuffer&
                        , const adcontrols::DataInterpreter& dataInterpreter
                        , unsigned long objId );

        // observer event handlers
        void handle_observer_config_changed( uint32_t objid, SignalObserver::eConfigStatus );
        void handle_observer_update_data( uint32_t objid, int32_t pos );
        void handle_observer_method_changed( uint32_t objid, int32_t pos );
        void handle_observer_event( uint32_t objid, int32_t pos, int32_t events );
        
        // receiver_i handlers
        // void handle_receiver_message( Receiver::eINSTEVENT, uint32_t );
        void handle_receiver_log( const ::EventLog::LogMessage& );
        void handle_receiver_shutdown();
        void handle_receiver_debug_print( int32_t, int32_t, std::string );

    };

}

#endif // ACQUIREPLUGIN_H
