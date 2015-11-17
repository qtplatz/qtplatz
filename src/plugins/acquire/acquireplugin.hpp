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

#include <thread>
#include <mutex>


//class QToolButton;
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
                                              
    public slots:

    private slots:
        void handle_shutdown();
        void handle_debug_print( unsigned long priority, unsigned long category, QString text );

        void handle_monitor_selected( int );
        void handle_monitor_activated( int );

        void handleSelected( const QPointF& );
        void handleSelected( const QRectF& );

        void handle_broker_initialized();

    signals:
        // 
    private:
        void selectPoint( double x, double y );
        void selectRange( double x1, double x2, double y1, double y2 );
        void handle_update_data( unsigned long objid, long pos );

#if HAVE_CORBA
        friend class orb_i;
        orb_i * orb_i_;
#endif
        std::mutex mutex_;

    };

}

#endif // ACQUIREPLUGIN_H
