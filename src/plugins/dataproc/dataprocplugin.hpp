// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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
#include "constants.hpp"
#include <vector>
#include <memory>

class QAction;

namespace adportable {
	class Configuration;
}

namespace adcontrols {
	class ProcessMethod;
}

namespace adextension {
    class iSequenceImpl;
}

namespace dataproc {

    class SessionManager;
    class MainWindow;
    class Mode;

    class DataprocManager;
    class ActionManager;
    class DataprocessorFactory;
    class iSnapshotHandlerImpl;
	class iPeptideHandlerImpl;

    class DataprocPlugin : public ExtensionSystem::IPlugin {
        Q_OBJECT
		Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Dataproc.json")
    public:
        ~DataprocPlugin();
        explicit DataprocPlugin();

        // implement ExtensionSystem::IPlugin
        bool initialize(const QStringList &arguments, QString *error_message) override;
        void extensionsInitialized() override;
        ShutdownFlag aboutToShutdown() override;

        // <--
        static DataprocPlugin * instance();
        static MainWindow * mainWindow();

        void applyMethod( const adcontrols::ProcessMethod& );
        ActionManager * actionManager();

    signals:
        void onApplyMethod( const adcontrols::ProcessMethod& );

    public slots:

    private:
        enum { idActSpectrogram, nActions };

    private:
        static DataprocPlugin * instance_;

        bool connect_isnapshothandler_signals();
        void disconnect_isnapshothandler_signals();

        class impl;
        std::unique_ptr< impl > impl_;
    };
}
