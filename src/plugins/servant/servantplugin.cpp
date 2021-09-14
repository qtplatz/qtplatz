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

#include "servantplugin.hpp"
#include "servantmode.hpp"
#include "outputwindow.hpp"
#include "logger.hpp"
#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <extensionsystem/pluginmanager.h>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrometer_factory.hpp>
#include <adlog/logging_handler.hpp>
#include <adlog/logger.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/plugin_ptr.hpp>
#include <adplugin_manager/manager.hpp>
#include <adplugin_manager/loader.hpp>
#include <adplugin/constants.hpp>
#include <adcontrols/logging_hook.hpp>
#include <adportfolio/logging_hook.hpp>
#include <adportable/debug.hpp>
#include <adportable/configuration.hpp>
#include <adportable/string.hpp>
#include <adportable/configloader.hpp>
#include <qtwrapper/application.hpp>

#include <QCoreApplication>
#include <QMessageBox>
#include <QtCore/qplugin.h>
#include <QtCore>
#include <qdebug.h>

#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <chrono>
#include <thread>
#if defined WIN32
#include <process.h>
#endif

using namespace servant;
using namespace servant::internal;

ServantPlugin * ServantPlugin::instance_ = 0;

ServantPlugin::~ServantPlugin()
{
    instance_ = 0;
#if ! defined NDEBUG && 1
    ADDEBUG() << "\t## DTOR ##";
#endif
}

ServantPlugin::ServantPlugin() : logger_(0)
                               , outputWindow_(0)
{
    instance_ = this;
}

ServantPlugin *
ServantPlugin::instance()
{
    return instance_;
}

bool
ServantPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);
	(void)error_message;

    adlog::logger::enable( adlog::logger::logging_file ); // process_name + ".log"

    if ( ( outputWindow_ = new OutputWindow ) ) {
        addAutoReleasedObject( outputWindow_ );

        if ( ( logger_ = new Logger ) ) {
            connect( logger_, SIGNAL( onLogging( const QString, bool ) ), outputWindow_, SLOT( handleLogging( const QString, bool ) ) );
            addAutoReleasedObject( logger_ );
            adlog::logging_handler::instance()->register_handler( std::ref(*logger_) );
        }
    }

    ADLOG(adlog::LOG_INFO) << "Startup " << QCoreApplication::applicationFilePath().toStdString();

    ///////////////////////////////////
    Core::Context context;
    context.add( Core::Id( "Servant.MainView" ) );
    context.add( Core::Id( Core::Constants::C_NAVIGATION_PANE ) );

    adplugin::manager::standalone_initialize();

    return true;
}

void
ServantPlugin::extensionsInitialized()
{
}

ExtensionSystem::IPlugin::ShutdownFlag
ServantPlugin::aboutToShutdown()
{
    adportable::core::debug_core::instance()->unhook();
    adcontrols::logging_hook::unregister_hook();
	adlog::logging_handler::instance()->close();

    if ( outputWindow_ && logger_ )
        disconnect( logger_, SIGNAL( onLogging( const QString, bool ) ), outputWindow_, SLOT( handleLogging( const QString, bool ) ) );

#if ! defined NDEBUG
    ADDEBUG() << "\t## Shutdown: "
              << "\t" << boost::filesystem::relative( boost::dll::this_line_location()
                                                     , boost::dll::program_location().parent_path() );
#endif

	return SynchronousShutdown;
}

Q_EXPORT_PLUGIN( ServantPlugin )
