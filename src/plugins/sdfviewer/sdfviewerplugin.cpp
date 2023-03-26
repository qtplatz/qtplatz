// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2023 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "sdfviewerplugin.hpp"
#include "sdfviewerfactory.hpp"
#include "sdfviewer.hpp"
#include "constants.hpp"
#include <adportable/debug.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>

#include <QAction>
#include <QCoreApplication>
#include <QKeySequence>

using namespace Core;
using namespace Utils;

namespace sdfviewer {

    class SDFViewer;

    class SDFViewerAction final : public QAction {
    public:
        SDFViewerAction(Id id,
                        const std::function<void(SDFViewer *v)> &onTriggered,
                        const QString &title = {},
                        const QKeySequence &key = {}) : QAction(title) {
            Command *command = ActionManager::registerAction(this, id, Context(sdfviewer::constants::SDFVIEWER_ID));
            if (!key.isEmpty())
                command->setDefaultKeySequence(key);

            connect(this, &QAction::triggered, this, [onTriggered] {
                if (auto iv = qobject_cast<SDFViewer *>(EditorManager::currentEditor()))
                    onTriggered(iv);
            });
        }
    };

    /////////////////

    class SDFViewerPlugin::impl {
    public:
        SDFViewerFactory factory_;
    };
}

using namespace sdfviewer;

SDFViewerPlugin::~SDFViewerPlugin()
{
}

SDFViewerPlugin::SDFViewerPlugin() : impl_( std::make_unique< impl >() )
{
    // ADDEBUG() << "## SDFViewerPlugin::ctor ##";
}

bool SDFViewerPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    return true;
}
