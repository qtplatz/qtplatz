// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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


#include "logger.hpp"
#include "servantplugin.hpp"
//#include <adinterface/brokerC.h>

#include <adplugin/manager.hpp>
#include <adorbmgr/orbmgr.hpp>
#include <acewrapper/constants.hpp>
//#include <acewrapper/brokerhelper.hpp>
#include <adportable/string.hpp>

///////-- ServantPlugin dependent -- ////////
#include "outputwindow.hpp"
#include <extensionsystem/pluginmanager.h>
#include <QMessageBox>
#include <boost/format.hpp>

using namespace servant;

namespace servant {   namespace internal {

        class LogHost {
            LogHost();
            ~LogHost();
            static LogHost * instance_;
        public:
            // Broker::Logger_var logger_;
            static LogHost * instance();
            static void initialize();
            static void shutdown();
        };
    }
}

///////

internal::LogHost * internal::LogHost::instance_ = 0;

internal::LogHost::LogHost()
{
}

internal::LogHost::~LogHost()
{
}

internal::LogHost *
internal::LogHost::instance()
{
	if ( ! LogHost::instance_ )
        initialize();
    return LogHost::instance_;
}

void
internal::LogHost::initialize()
{
    ServantPlugin * plugin = ServantPlugin::instance();
    if ( plugin ) {
		Broker::Manager_var manager = adorbmgr::orbmgr::getBrokerManager();
        if ( ! CORBA::is_nil( manager.in() ) ) {
            LogHost::instance_ = new LogHost();
            LogHost::instance_->logger_ = manager->getLogger();
        }
    }
}

void
internal::LogHost::shutdown()
{
    delete LogHost::instance_;
}

////////////////////////////
///////////////////////////

Logger::Logger( const std::wstring& srcid ) : srcid_(srcid)
{
}

Logger::~Logger(void)
{
    internal::LogHost * host = internal::LogHost::instance();
    if ( host ) {
        Broker::LogMessage log;
        log.priority = 0;
        log.srcId = L"Servant";
        log.text = CORBA::wstring_dup( stream_.str().c_str() );
        // log.args;
        log.tv_sec = 0;
        log.tv_usec = 0;
        host->logger_->log( log );
    } else {
		// all logs before Broker servant initialized to be here
        OutputWindow * outputWindow = ExtensionSystem::PluginManager::instance()->getObject< servant::OutputWindow >();
        outputWindow->appendLog( stream_.str() );
    }
}

void
Logger::initialize()
{
    internal::LogHost::initialize();
}

void
Logger::shutdown()
{
    internal::LogHost::shutdown();
}

void
Logger::operator()( const std::string& text )
{
    stream_ << adportable::string::convert( text );
}

void
Logger::operator()( const std::wstring& text )
{
    stream_ << text;
}
