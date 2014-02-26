// -*- C++ -*-
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

#include "u5303aplugin.hpp"
#include "u5303a_constants.hpp"
#include "u5303amode.hpp"
#include "mainwindow.hpp"

#include <adportable/debug_core.hpp>
#include <adlog/logging_handler.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/modemanager.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>

using namespace u5303a::Internal;
using namespace u5303a;

u5303APlugin::u5303APlugin() : mainWindow_( new MainWindow() )
                             , mode_( std::make_shared< u5303AMode >(this) )
{
    // Create your members
}

u5303APlugin::~u5303APlugin()
{
    // Unregister objects from the plugin manager's object pool
    // Delete members
    removeObject( mode_.get() );
}

bool u5303APlugin::initialize(const QStringList &arguments, QString *errorString)
{
    adportable::core::debug_core::instance()->hook( adlog::logging_handler::log );

    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
    mainWindow_->activateWindow();
    mainWindow_->createActions();

	const QList<int> gc = QList<int>() << Core::Constants::C_GLOBAL_ID;
    mode_->setContext( gc );
    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
        mode_->setWidget( widget );
    addObject( mode_.get() );

    QAction *action = new QAction(tr("u5303A action"), this);

    QList<int> globalcontext;
    globalcontext << Core::Constants::C_GLOBAL_ID;

	Core::ActionManager * am = Core::ICore::instance()->actionManager();
    Core::Command * cmd = am->registerAction(action, Constants::ACTION_ID, globalcontext );
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));

    Core::ActionContainer *menu = am->createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("U5303A"));
    menu->addAction(cmd);
    am->actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);

    return true;
}

void u5303APlugin::extensionsInitialized()
{
	mainWindow_->OnInitialUpdate();
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized function, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

ExtensionSystem::IPlugin::ShutdownFlag u5303APlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    return SynchronousShutdown;
}

void u5303APlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from u5303a."));
}

Q_EXPORT_PLUGIN2(u5303a, u5303APlugin)

