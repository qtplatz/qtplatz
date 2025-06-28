/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "chemistryplugin.hpp"
#include "chemistryconstants.hpp"
#include "mode.hpp"
#include "mainwindow.hpp"
#include "constants.hpp"
#include <adportable/debug.hpp>
#include <adportable/scoped_debug.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>

#include <coreplugin/modemanager.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <extensionsystem/pluginmanager.h>
#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QStackedWidget>
#include <QTextEdit>
#include <QWidget>
#include <boost/dll.hpp>
#include <memory>

namespace chemistry {

    class ChemistryPlugin::impl {
    public:
        impl() {}
        ~impl() {}
        std::unique_ptr< Mode > mode_;
        std::unique_ptr< MainWindow > mainWindow_;
    };
}

using namespace chemistry;

ChemistryPlugin::ChemistryPlugin() : impl_( new impl() )
{
}

ChemistryPlugin::~ChemistryPlugin()
{
    delete impl_;
}

bool
ChemistryPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    initialize_actions();

    if (( impl_->mainWindow_ = std::make_unique< MainWindow >() )) {
        impl_->mainWindow_->activateWindow();
        impl_->mainWindow_->createActions();

        if ( QWidget * widget = impl_->mainWindow_->createContents( /* mode_.get() */ ) ) {
            if (( impl_->mode_ = std::make_unique< Mode >() )) {
                impl_->mode_->setWidget( widget );
                // ExtensionSystem::PluginManager::addObject( mode_.get() );
            }
        }
    }
    return true;
}

void
ChemistryPlugin::initialize_actions()
{
    Core::Context gc( "Chemistry" );

	if ( Core::ActionManager *am = Core::ActionManager::instance() ) {

        // File->Processing
        if ( Core::ActionContainer * menu = am->createMenu( "Chemistry.menu" ) ) {
            do {
                QIcon iconOpen;
                iconOpen.addFile( ":/dataproc/image/fileopen.png" );
                if ( auto p = new QAction( iconOpen, tr("SDFile open..."), this ) ) {
                    am->registerAction( p, Constants::SDFILE_OPEN, gc );
                    connect( p, &QAction::triggered, impl_->mainWindow_.get(), &MainWindow::actSDFileOpen );
                }
            } while ( 0 );

            menu->menu()->setTitle( tr("Chemistry") );
            menu->addAction( am->command( Constants::SDFILE_OPEN ) );
            am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );
        }

    }
}

void
ChemistryPlugin::extensionsInitialized()
{
	impl_->mainWindow_->OnInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
ChemistryPlugin::aboutToShutdown()
{
	impl_->mainWindow_->OnClose();
	return SynchronousShutdown;
}

void
ChemistryPlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from Chemistry."));
}
