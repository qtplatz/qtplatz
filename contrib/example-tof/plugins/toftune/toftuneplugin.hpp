// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#pragma once

#include "toftune_global.hpp"
#include <tofinterface/tofC.h>
#include <adinterface/brokerC.h>
#include <adinterface/signalobserverC.h>
#include <extensionsystem/iplugin.h>
#include <map>
#include <memory>
#include <vector>

namespace adwplot {
    class ChromatogramWidget;
    class SpectrumWidget;
}

namespace adcontrols {
    class Trace;
}

namespace adplugin { class QObserverEvents_i; }
namespace EventLog { struct LogMessage; }
namespace tof { class ControlMethod; }

namespace toftune {

    class MainWindow;
    class SideFrame;
    class tofTuneMode;
    class iSequenceImpl;
    
    namespace Internal {
        
        template<class T> class Receiver_i;
        
        class tofTunePlugin : public ExtensionSystem::IPlugin {
            Q_OBJECT
			Q_PLUGIN_METADATA(IID "com.ms-cheminfo.qtplatzplugin" FILE "toftune.json")
                static tofTunePlugin * instance_;
            
        public:
            tofTunePlugin();
            ~tofTunePlugin();
            
            static tofTunePlugin * instance();
            
            // IPlugin
            bool initialize(const QStringList &arguments, QString *errorString) override;
            void extensionsInitialized() override;
            ShutdownFlag aboutToShutdown() override;
            
            void setMethod( const tof::ControlMethod&, const std::string& );
            
        private:
            friend class Receiver_i< tofTunePlugin >;
            void onMessage( unsigned long msg, unsigned long value);
            void onLog( const EventLog::LogMessage& );
            void onPrint( long pri, long cat, const char * text );

        public slots:
            void actionConnect();

        private slots:
            void triggerAction();
            void HandleUpdateData( unsigned long objId, long pos );
            void HandleMessage( unsigned long msg, unsigned long value );
            void HandleLog( QString, QString );
            
        signals:
            void OnMessage( unsigned long, unsigned long );
            void OnLog( QString, QString );
                
        private:
            std::unique_ptr< tofTuneMode > mode_;
            std::unique_ptr< MainWindow > mainWindow_;
            
            Broker::Session_var brokerSession_;
            std::unique_ptr< adplugin::QObserverEvents_i > signalObserverEvents_i_;
            Receiver_i< Internal::tofTunePlugin > * receiver_i_;
            SignalObserver::Observer_var signalObserver_;
            bool hasController_;
            std::vector< std::shared_ptr< adcontrols::Trace > > traces_;
            std::unique_ptr< iSequenceImpl > iSequenceImpl_;
        };
        
    } // namespace Internal
} // namespace toftune



