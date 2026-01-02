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

#include "queryplugin.hpp"
#include "mainwindow.hpp"
#include "queryconstants.hpp"
#include "queryfactory.hpp"
#include "querymode.hpp"
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

namespace query {

    class QueryPlugin::impl {
    public:
        impl() : editorFactory_( std::make_unique< QueryFactory >() ) {
        }
        void ini() {
            if (( mode_ = std::make_unique< QueryMode >() )) {
                if (( mainWindow_ = std::make_unique< MainWindow >() )) {
                    mainWindow_->createActions();
                    mode_->setWidget( mainWindow_->createContents( mode_.get() ) );
                }
            }
        }
        void fin() {
            mainWindow_->onFinalClose();
        }

        std::unique_ptr< QueryMode > mode_;
        std::unique_ptr< MainWindow > mainWindow_;
        std::unique_ptr< QueryFactory > editorFactory_; // self-reg
    };
}

using namespace query;

QueryPlugin::QueryPlugin() : impl_( std::make_unique< impl >() )

{
}

QueryPlugin::~QueryPlugin()
{
    // if ( mode_ )
    //     removeObject( mode_.get() );
    // mainWindow has been deleted at BaseMode dtor
#if ! defined NDEBUG
    ADDEBUG() << "\t## DTOR ##";
#endif
}

Utils::Result<> // bool
QueryPlugin::initialize(const QStringList &arguments)
{
    Q_UNUSED(arguments);

    impl_->ini();

    return Utils::ResultOk;
}

void QueryPlugin::extensionsInitialized()
{
    impl_->mainWindow_->onInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
QueryPlugin::aboutToShutdown()
{
    impl_->fin();
    // Save settings
    // Disconnect from signals that are not needed during shutdown
    // Hide UI (if you add UI that is not in the main window directly)
#if ! defined NDEBUG
    ADDEBUG() << "\t## Shutdown: "
              << "\t" << boost::filesystem::relative( boost::dll::this_line_location()
                                                     , boost::dll::program_location().parent_path() );
#endif
    return SynchronousShutdown;
}

#if QTC_VERSION <= 0x03'02'81
Q_EXPORT_PLUGIN2(Query, QueryPlugin)
#endif
