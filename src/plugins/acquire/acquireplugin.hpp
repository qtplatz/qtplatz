// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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
#if ! defined Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#endif
#include <vector>
#include <map>

#  include <adinterface/controlserverC.h>
#  include <adinterface/signalobserverC.h>

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

namespace Acquire {
    namespace internal {

        class AcquireUIManager;
        class AcquireImpl;
        class ObserverEvents_i;

        //------------
        class AcquirePlugin : public ExtensionSystem::IPlugin {
            Q_OBJECT
			Q_PLUGIN_METADATA(IID "com.ms-cheminfo.QtPlatzPlugin" FILE "acquire.json")
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
        // 
        private:
            void selectPoint( double x, double y );
            void selectRange( double x1, double x2, double y1, double y2 );

            AcquireUIManager * manager_;
            AcquireImpl * pImpl_;

            QAction * actionConnect_;
            QAction * actionRun_;
            QAction * actionInitRun_;
            QAction * actionStop_;
            QAction * actionSnapshot_;
            QAction * actionInject_;

            void initialize_actions();

            ControlServer::Session_var session_;
            SignalObserver::Observer_var observer_;
            std::map< unsigned long, SignalObserver::Observer_var > observerMap_;
            std::map< unsigned long, boost::shared_ptr< adcontrols::MassSpectrum > > rdmap_;

            boost::scoped_ptr< adplugin::QReceiver_i > receiver_i_;
            boost::scoped_ptr< adplugin::QObserverEvents_i > masterObserverSink_;
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

        public:
            static QToolButton * toolButton( QAction * action );
        };
    }
}

#endif // ACQUIREPLUGIN_H
