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

#include "peptideplugin.hpp"
#include "peptideconstants.hpp"
#include "mainwindow.hpp"
#include "peptidemode.hpp"
#include "utils/result.h"

#include <adportable/debug_core.hpp>
#include <adlog/logging_handler.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#if QTC_VERSION <= 0x03'02'81
#include <coreplugin/id.h>
#else
#include <utils/id.h>
#endif
#include <coreplugin/modemanager.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QtPlugin>
#include <adportable/debug.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem/path.hpp>

namespace {
    static constexpr char __CLASS_NAME__[] = "peptideplugin";
}

namespace peptide {

    class peptideplugin::impl {
    public:
        impl() {}
        ~impl() {}

        void ini() {
            if (( mainWindow_ = std::make_unique< MainWindow >() )) {
                mainWindow_->activateWindow();
                mainWindow_->createActions();
                if (( mode_ = std::make_unique< PeptideMode >() )) {
                    if ( QWidget * widget = mainWindow_->createContents( mode_.get() ) )
                        mode_->setWidget( widget );
                }
            }
        }
        void fin() {
        }
        std::unique_ptr< MainWindow > mainWindow_;
        std::unique_ptr< PeptideMode > mode_;
    };
}

using namespace peptide;

peptideplugin::peptideplugin() : impl_( std::make_unique< impl >() )
{
}

peptideplugin::~peptideplugin()
{
}

Utils::Result<>
peptideplugin::initialize(const QStringList &arguments)
{
    (void)arguments;

#if ! defined NDEBUG
    ADDEBUG() << "\t#### " << __CLASS_NAME__ << "::" << __FUNCTION__ << " ####";
#endif
    impl_->ini();

    return Utils::ResultOk;
}

void peptideplugin::extensionsInitialized()
{
#if ! defined NDEBUG
    ADDEBUG() << "\t#### " << __CLASS_NAME__ << "::" << __FUNCTION__ << " ####";
#endif
    impl_->mainWindow_->onInitialUpdate();
}

ExtensionSystem::IPlugin::ShutdownFlag
peptideplugin::aboutToShutdown()
{
    impl_->fin();

#if ! defined NDEBUG
    ADDEBUG() << "\t\t## " << __CLASS_NAME__ << "::" << __FUNCTION__
              << "\t" << std::filesystem::relative( boost::dll::this_line_location()
                                                     , boost::dll::program_location().parent_path() );
#endif

    return SynchronousShutdown;
}

void
peptideplugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from peptide."));
}
