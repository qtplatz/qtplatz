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
#if QTC_VERSION <= 0x03'02'81
#include <coreplugin/id.h>
#endif
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
#include <QDebug>

#include <boost/dll.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <chrono>
#include <thread>
#if defined WIN32
#include <process.h>
#endif

namespace servant {

    class ServantPlugin::impl {
    public:
        impl() {}

        static void qDebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
            adportable::debug( context.file, context.line ) << msg.toStdString();
        }

        void ini() {
            if (( outputWindow_ = std::make_unique< OutputWindow >() )) {
                if (( logger_ = std::make_unique< Logger >() )) {
                    QObject::connect( logger_.get(), SIGNAL( onLogging( const QString, bool ) )
                                      , outputWindow_.get(), SLOT( handleLogging( const QString, bool ) ) );
                    adlog::logging_handler::instance()->register_handler( std::ref(*logger_) );
                }
            }
            qInstallMessageHandler(qDebugHandler);
            // qSetMessagePattern("[%{file}:%{line}] - %{message}"); // <-- this will be overloaded by qDebugHelper
        }

        void fin() {
        }

        std::unique_ptr< OutputWindow > outputWindow_;
        std::unique_ptr< Logger > logger_;
    };

}

using namespace servant;
using namespace servant::internal;

ServantPlugin::~ServantPlugin()
{
    ADDEBUG() << "------------- ServantPlugin dtor ----------------";
}

ServantPlugin::ServantPlugin() : impl_( std::make_unique< impl >() )
{
    ADDEBUG() << "------------- ServantPlugin ctor ----------------";
}

bool
ServantPlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);
	(void)error_message;

    adlog::logger::enable( adlog::logger::logging_file ); // process_name + ".log"

    impl_->ini();

    ADLOG(adlog::LOG_INFO) << "Startup " << QCoreApplication::applicationFilePath().toStdString();

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
    impl_->fin();
    adportable::core::debug_core::instance()->unhook();
    adcontrols::logging_hook::unregister_hook();
	adlog::logging_handler::instance()->close();

    ADDEBUG() << "\t------------- servantplugin ---------- Shutdown: "
              << "\t" << boost::filesystem::relative( boost::dll::this_line_location()
                                                      , boost::dll::program_location().parent_path() );

	return SynchronousShutdown;
}

#if QTC_VERSION < 0x08'00'00
Q_EXPORT_PLUGIN( ServantPlugin );
#endif
