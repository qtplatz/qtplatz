// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <map>

#include <adinterface/controlserverC.h>
#include <adinterface/signalobserverC.h>

class QToolButton;
class QAction;
class QComboBox;
class QPointF;
class QRectF;

namespace adplugin {
   class QReceiver_i;
   class QObserverEvents_i;
}

namespace adcontrols {
    class MassSpectrometer;
    class DataInterpreter;
    class MassSpectrum;
}

namespace adportable {
    class Configuration;
}

namespace adinterface {
    class ObserverEvents_i;
}


namespace Acquire {

    namespace internal {

        class MainWindow;
        class AcquireImpl;
        class ObserverEvents_i;

        //------------
        class AcquirePlugin : public ExtensionSystem::IPlugin {
            Q_OBJECT
			Q_PLUGIN_METADATA(IID "com.ms-cheminfo.qtplatz.plugin" FILE "acquire.json")
        public:
            ~AcquirePlugin();
            AcquirePlugin();

            // implement IPlugin
            virtual bool initialize(const QStringList &arguments, QString *error_message);
            virtual void extensionsInitialized();
            virtual ShutdownFlag aboutToShutdown();

        private slots:
            void actionConnect();
            void actionDisconnect();
            void actionSnapshot();
            void actionInitRun();
            void actionRun();
            void actionStop();
            void actionInject();

            void handle_message( unsigned long msg, unsigned long value );
            void handle_log( QByteArray );
            void handle_shutdown();
            void handle_debug_print( unsigned long priority, unsigned long category, QString text );

            void handle_config_changed( unsigned long objid, long pos );
            void handle_update_data( unsigned long objid, long pos );
            void handle_method_changed( unsigned long objid, long pos );
			void handle_event( unsigned long objid, unsigned long, long pos );

            void handle_monitor_selected( int );
            void handle_monitor_activated( int );

			void handleSelected( const QPointF& );
			void handleSelected( const QRectF& );

        signals:
            void onObsConfigChanged( unsigned long, long );
            void onObsUpdateData( unsigned long, long );
            void onObsMethodChanged( unsigned long, long );
            void onObsEvent( unsigned long, long, long );
        // 
        private:
            void selectPoint( double x, double y );
            void selectRange( double x1, double x2, double y1, double y2 );

            MainWindow * mainWindow_;
            AcquireImpl * pImpl_;

            QAction * actionConnect_;
            QAction * actionRun_;
            QAction * actionInitRun_;
            QAction * actionStop_;
            QAction * actionSnapshot_;
            QAction * actionInject_;
            adportable::Configuration * pConfig_;

            void initialize_actions();

            ControlServer::Session_var session_;
            SignalObserver::Observer_var observer_;
            std::map< unsigned long, SignalObserver::Observer_var > observerMap_;
            std::map< unsigned long, std::shared_ptr< adcontrols::MassSpectrum > > rdmap_;

            std::unique_ptr< adplugin::QReceiver_i > receiver_i_;
            std::unique_ptr< adinterface::ObserverEvents_i > sink_;
            std::vector< std::wstring > trace_descriptions_;
            QComboBox * traceBox_;
            void populate( SignalObserver::Observer_var& );

            void readMassSpectra( const SignalObserver::DataReadBuffer&
                                  , const adcontrols::MassSpectrometer&
                                  , const adcontrols::DataInterpreter& dataInterpreter
                                  , unsigned long objId );
            void readTrace( const SignalObserver::Description&
                , const SignalObserver::DataReadBuffer&
                , const adcontrols::DataInterpreter& dataInterpreter );

            // observer event handlers
            void handle_observer_config_changed( uint32_t objid, SignalObserver::eConfigStatus );
            void handle_observer_update_data( uint32_t objid, int32_t pos );
            void handle_observer_method_changed( uint32_t objid, int32_t pos );
            void handle_observer_event( uint32_t objid, int32_t pos, int32_t events );

        public:
            static QToolButton * toolButton( QAction * action );
        };
    }
}

#endif // ACQUIREPLUGIN_H
