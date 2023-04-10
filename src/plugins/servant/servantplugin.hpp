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
#include <memory>

namespace adportable {
    class Configuration;
}

namespace servant {

    class ServantPlugin : public ExtensionSystem::IPlugin {
	    Q_OBJECT
        Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QtCreatorPlugin" FILE "Servant.json")

    public:
        explicit ServantPlugin();
        ~ServantPlugin();

        // ExtensionSystem::IPlugin
        bool initialize(const QStringList &arguments, QString *error_message) override;
        void extensionsInitialized() override;
        ShutdownFlag aboutToShutdown() override;

    signals:

    public slots:

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };
}
