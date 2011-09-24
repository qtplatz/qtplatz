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

namespace Broker {
    class Session;
}

namespace dataproc {

    class SessionManager;

    namespace internal {

        class DataprocManager;
        class ActionManager;

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
            SessionManager * getSessionManager();

            static DataprocPlugin * instance();

        signals:

        public slots:
            void actionApply();

        private slots:
            void handleFeatureSelected( int );
            void handleFeatureActivated( int );
            void handle_portfolio_created( const QString token );
            void handle_folium_added( const QString, const QString, const QString );

        private:

        private:
            boost::shared_ptr<DataprocManager> manager_;
            boost::shared_ptr< adportable::Configuration > pConfig_;
            boost::scoped_ptr< SessionManager > pSessionManager_;
            boost::scoped_ptr< ActionManager > pActionManager_;

            QBrokerSessionEvent * pBrokerSessionEvent_;
            Broker::Session * brokerSession_;

            QAction * actionApply_;
            enum ProcessType currentFeature_;

            static DataprocPlugin * instance_;

        };

    }
}

