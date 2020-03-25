/**************************************************************************
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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

#include "clusterfactory.hpp"
#include "clusterplugin.hpp"
#include "constants.hpp"
#include "document.hpp"
#include "mainwindow.hpp"
#include "mode.hpp"
#include <adextension/isessionmanager.hpp>
#include <adportable/debug.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <extensionsystem/pluginmanager.h>

#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>


using namespace cluster;

ClusterPlugin::ClusterPlugin() : mode_( std::make_shared<Mode>( this ) )
                               , mainWindow_( new MainWindow() )
{
}

ClusterPlugin::~ClusterPlugin()
{
    if ( mode_ )
        removeObject( mode_.get() );
    // mainWindow has been deleted at BaseMode dtor
}

bool
ClusterPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

    mainWindow_->createActions();

    mode_->setId(  Constants::C_CLUSTER_MODE ); // "cluster.Mode"
    mode_->setContext( Core::Context( Constants::C_CLUSTER_MODE ) );

    mode_->setWidget( mainWindow_->createContents( mode_.get() ) );

    addObject( mode_.get() );

    return true;
}

void ClusterPlugin::extensionsInitialized()
{
    auto vec = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSessionManager >();
    for ( auto mgr: vec ) {
        using adextension::iSessionManager;
        connect( mgr, &iSessionManager::addProcessor, document::instance(), &document::handleAddProcessor );
        connect( mgr, &iSessionManager::onSelectionChanged, document::instance(), &document::handleSelectionChanged );
        connect( mgr, &iSessionManager::onProcessed, document::instance(), &document::handleProcessed );
        connect( mgr, &iSessionManager::onCheckStateChanged, document::instance(), &document::handleCheckStateChanged );
    }
    document::instance()->initialSetup();
    mainWindow_->onInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag ClusterPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    mainWindow_->onFinalClose();
    document::instance()->finalClose();
    return SynchronousShutdown;
}

Q_EXPORT_PLUGIN2(CLUSTER, ClusterPlugin)
