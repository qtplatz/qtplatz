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

#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>
#include <vector>
#include "constants.hpp"

class QAction;
class QBrokerSessionEvent;

namespace adportable {
	class Configuration;
}

namespace adcontrols {
	class ProcessMethod;
}

namespace Broker {
    class Session;
}

namespace dataproc {

    class SessionManager;
    class EditorFactory;
    class MainWindow;
    class Mode;

    class DataprocManager;
    class ActionManager;
    class DataprocessorFactory;
    
    class DataprocPlugin : public ExtensionSystem::IPlugin {
        
        Q_OBJECT
    public:
        ~DataprocPlugin();
        explicit DataprocPlugin();
        
        // implement ExtensionSystem::IPlugin
        bool initialize(const QStringList &arguments, QString *error_message);
        void extensionsInitialized();
        void shutdown();
        // <--
        inline static DataprocPlugin * instance() { return instance_; }
        
        void applyMethod( const adcontrols::ProcessMethod& );
        void onSelectTimeRangeOnChromatogram( double x1, double x2 );
        DataprocessorFactory * dataprocessorFactory() { return dataprocFactory_; }
        
    signals:
        void onApplyMethod( const adcontrols::ProcessMethod& );
                                                              
    public slots:
        // void actionApply();
            
    private slots:
        //void handleFeatureSelected( int );
        //void handleFeatureActivated( int );
        void handle_portfolio_created( const QString token );
        void handle_folium_added( const QString, const QString, const QString );
        
    private:
        
    private:
        boost::shared_ptr<DataprocManager> manager_;
        dataproc::MainWindow * mainWindow_;
        boost::scoped_ptr< dataproc::Mode > mode_;
        boost::shared_ptr< adportable::Configuration > pConfig_;
        boost::scoped_ptr< SessionManager > pSessionManager_;
        boost::scoped_ptr< ActionManager > pActionManager_;
        
        QBrokerSessionEvent * pBrokerSessionEvent_;
        Broker::Session * brokerSession_;
        DataprocessorFactory * dataprocFactory_;
        
        // QAction * actionApply_;
        std::vector< EditorFactory * > factories_;
        static DataprocPlugin * instance_;
        
        static void install_dataprovider( const adportable::Configuration&, const std::wstring& );
        static void install_editorfactories( const adportable::Configuration&, const std::wstring&, std::vector< EditorFactory * >& );
        static void delete_editorfactories( std::vector< EditorFactory * >& );
    };
}

