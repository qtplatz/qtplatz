/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "frequencyplugin.hpp"
#include "frequencyconstants.hpp"
#include "mode.hpp"
#include "mainwindow.hpp"

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <coreplugin/mimedatabase.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>

using namespace frequency;

frequencyPlugin::frequencyPlugin()
{
    // Create your members
}

frequencyPlugin::~frequencyPlugin()
{
    if ( mode_ )
        removeObject( mode_.get() );
}

bool frequencyPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    (void)arguments;
    (void)errorString;

	mode_.reset( new Mode( this ) );
	if ( ! mode_ )
		return false;

#if 0
    // Register objects in the plugin manager's object pool
    // Load settings
    // Add actions to menus
    // Connect to other plugins' signals
    // In the initialize method, a plugin can be sure that the plugins it
    // depends on have initialized their members.
    
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)
    
    QAction *action = new QAction(tr("frequency action"), this);
    Core::Command *cmd = Core::ActionManager::registerAction(action, Constants::ACTION_ID,
                                                             Core::Context(Core::Constants::C_GLOBAL));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Alt+Meta+A")));
    connect(action, SIGNAL(triggered()), this, SLOT(triggerAction()));
    
    Core::ActionContainer *menu = Core::ActionManager::createMenu(Constants::MENU_ID);
    menu->menu()->setTitle(tr("frequency"));
    menu->addAction(cmd);
    Core::ActionManager::actionContainer(Core::Constants::M_TOOLS)->addMenu(menu);
#endif    

    mainWindow_.reset( new MainWindow() );
    if ( ! mainWindow_ )
        return false;

    Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
    mainWindow_->activateLayout();
    mainWindow_->createActions();
    QWidget * widget = mainWindow_->createContents( mode_.get() );
    mode_->setWidget( widget );
    addObject( mode_.get() );

    return true;
}

void frequencyPlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // In the extensionsInitialized method, a plugin can be sure that all
    // plugins that depend on it are completely initialized.
}

void frequencyPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from Chemistry."));
}

Q_EXPORT_PLUGIN2(frequency, frequencyPlugin)

