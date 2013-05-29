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

#pragma once

#include <extensionsystem/iplugin.h>
#include <vector>
#include "constants.hpp"
#if ! defined Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#endif

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
    class iSequenceImpl;
    
    class DataprocPlugin : public ExtensionSystem::IPlugin {
        
        Q_OBJECT
        Q_PLUGIN_METADATA(IID "com.ms-cheminfo.QtPlatzPlugin" FILE "dataproc.json")

    public:
        ~DataprocPlugin();
        explicit DataprocPlugin();
        
        // implement ExtensionSystem::IPlugin
        bool initialize(const QStringList &arguments, QString *error_message);
        void extensionsInitialized();
        void shutdown();
        // <--
        inline static DataprocPlugin * instance() { return instance_; }
        inline static MainWindow * mainWindow() { return instance_ ? instance_->mainWindow_ : 0; }
        
        void applyMethod( const adcontrols::ProcessMethod& );
        void onSelectTimeRangeOnChromatogram( double x1, double x2 );
        DataprocessorFactory * dataprocessorFactory() { return dataprocFactory_; }
        
    signals:
        void onApplyMethod( const adcontrols::ProcessMethod& );
                                                              
    public slots:
            
    private slots:
        void handle_portfolio_created( const QString token );
        void handle_folium_added( const QString, const QString, const QString );
        
    private:
        
    private:
        dataproc::MainWindow * mainWindow_;
        boost::scoped_ptr< dataproc::Mode > mode_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
        boost::scoped_ptr< SessionManager > pSessionManager_;
        boost::scoped_ptr< ActionManager > pActionManager_;
        
        QBrokerSessionEvent * pBrokerSessionEvent_;
        Broker::Session * brokerSession_;
        DataprocessorFactory * dataprocFactory_;
        
        boost::scoped_ptr< iSequenceImpl > iSequence_;
        static DataprocPlugin * instance_;
        
        static bool install_dataprovider( const adportable::Configuration&, const std::wstring& );
        static bool install_isequence( const adportable::Configuration&, const std::wstring&, iSequenceImpl& );
        static void delete_editorfactories( std::vector< EditorFactory * >& );
    };
}

