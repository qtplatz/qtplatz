//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "logger.h"
#pragma warning(disable:4996)
#include <adinterface/brokerC.h>
#pragma warning(default:4996)
#include <adplugin/adplugin.h>
#include <adplugin/orbmanager.h>
#include <acewrapper/constants.h>
#include <acewrapper/brokerhelper.h>
#include <adportable/string.h>
///////-- ServantPlugin dependent -- ////////
#include "outputwindow.h"
#include <extensionsystem/pluginmanager.h>
#include <QMessageBox>
#include <boost/format.hpp>

using namespace servant;

namespace servant {
    namespace internal {

        class LogHost {
            LogHost();
            ~LogHost();
            static LogHost * instance_;
        public:
            Broker::Logger_var logger_;
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
    std::string ior = adplugin::manager::instance()->iorBroker(); 
    CORBA::Object_var obj = adplugin::ORBManager::instance()->string_to_object( ior );

    try {
        Broker::Manager_var manager = Broker::Manager::_narrow( obj.in() );
        if ( ! CORBA::is_nil( manager.in() ) ) {
            LogHost::instance_ = new LogHost();
            if ( LogHost::instance_ )
                LogHost::instance_->logger_ = manager->getLogger();
        }
    } catch ( CORBA::Exception& src) {
        QMessageBox mbx;
        mbx.critical( 0, "Servant error", QString("LogHost initialize got a CORBA::Exception: ") 
            + ( boost::format("%1% : %2%") % src._name() % src._info()).str().c_str() );
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
        // all logs before (or failed) to create Broker servant
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
