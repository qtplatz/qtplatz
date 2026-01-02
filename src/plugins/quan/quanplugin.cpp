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
#include "mainwindow.hpp"
#include "quanconstants.hpp"
#include "quanfactory.hpp"
#include "quanmode.hpp"
#include "utils/result.h"
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
#include <adportable/debug.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem/path.hpp>


namespace quan {

    class QuanPlugin::impl {
    public:
        impl() : editorFactory_( std::make_unique< QuanFactory >() )
               , mainWindow_( std::make_unique< MainWindow >() ) {}
        void ini() {
            if (( mode_ = std::make_unique< QuanMode >() )) {
                mainWindow_->createActions();
                mode_->setWidget( mainWindow_->createContents( mode_.get() ) );
            }
        }
        void fin() {
            mainWindow_->onFinalClose();
        }
        std::unique_ptr< QuanFactory > editorFactory_; // self-reg
        std::unique_ptr< MainWindow > mainWindow_;
        std::unique_ptr< QuanMode > mode_;
    };

}

using namespace quan;

QuanPlugin::QuanPlugin() : impl_( std::make_unique< impl >() )
{
}

QuanPlugin::~QuanPlugin()
{
    // if ( mode_ )
    //     removeObject( mode_.get() );
    // mainWindow has been deleted at BaseMode dtor
#if ! defined NDEBUG
    ADDEBUG() << "\t\t## DTOR ##";
#endif
}

Utils::Result<>
QuanPlugin::initialize(const QStringList &arguments)
{
    Q_UNUSED(arguments);

    impl_->ini();

    return Utils::ResultOk;
}

void QuanPlugin::extensionsInitialized()
{
    impl_->mainWindow_->onInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
QuanPlugin::aboutToShutdown()
{
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
    impl_->fin();

#if ! defined NDEBUG
    ADDEBUG() << "\t\t## Shutdown: "
              << "\t" << boost::filesystem::relative( boost::dll::this_line_location()
                                                     , boost::dll::program_location().parent_path() );
#endif

    return SynchronousShutdown;
}

#if QTC_VERSION <= 0x03'02'82
Q_EXPORT_PLUGIN2(Quan, QuanPlugin)
#endif
