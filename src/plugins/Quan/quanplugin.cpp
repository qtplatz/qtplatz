/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "quanplugin.hpp"
#include "quanconstants.hpp"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>

using namespace Quan::Internal;

QuanPlugin::QuanPlugin()
{
    // Create your members
}

QuanPlugin::~QuanPlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
}

bool QuanPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    QAction *action = new QAction(tr("Quan action"), this);
    auto am = Core::ICore::instance()->actionManager();

    const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;

    Core::Command *cmd = am->registerAction( action, Constants::ACTION_ID, gc ); // Core::Context( Core::Constants::C_GLOBAL ) );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

    Core::ActionContainer *menu = am->createMenu( Constants::MENU_ID );
    menu->menu()->setTitle(tr("Quan"));
    menu->addAction(cmd);
    am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );

    return true;
}

void QuanPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag QuanPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

void QuanPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from Quan."));
}

Q_EXPORT_PLUGIN2(Quan, QuanPlugin)

