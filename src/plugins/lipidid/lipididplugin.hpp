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

namespace lipidid {

    class MainWindow;
    class Mode;
    class ActionManager;

    class LipididPlugin : public ExtensionSystem::IPlugin {
        Q_OBJECT
		Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "LipidId.json")

    public:
        ~LipididPlugin();
        explicit LipididPlugin();

        // implement ExtensionSystem::IPlugin
        Utils::Result<> initialize(const QStringList &arguments) override;
        void extensionsInitialized() override;
        ShutdownFlag aboutToShutdown() override;

        // <--
        void applyMethod( const adcontrols::ProcessMethod& );

    signals:

    public slots:

    private:

    private:
        MainWindow * mainWindow_;
        std::unique_ptr< lipidid::Mode > mode_;
        bool connect_isnapshothandler_signals();
        void disconnect_isnapshothandler_signals();
    };
}
