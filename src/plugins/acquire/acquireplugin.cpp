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

#include "acquireplugin.hpp"
#include "acquiremode.hpp"
#include "constants.hpp"
#include "document.hpp"
#include "mastercontroller.hpp"
#include "mainwindow.hpp"

#if HAVE_CORBA
#include "orb_i.hpp"
#include "orbconnection.hpp"
#include "qbroker.hpp"
#endif

#include <acewrapper/constants.hpp>
#include <acewrapper/ifconfig.hpp>
#include <adextension/iacquire.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adcontrols/samplerun.hpp>
#include <adportable/debug.hpp>
#include <adportable/debug_core.hpp>
#include <adplugin_manager/loader.hpp>
#include <adlog/logging_handler.hpp>
#include <adlog/logger.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/id.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/modemanager.h>
#include <extensionsystem/pluginmanager.h>
#include <QAction>
#include <QtCore/qplugin.h>
#include <QMessageBox>
#include <boost/exception/all.hpp>

using namespace acquire;

AcquirePlugin::~AcquirePlugin()
{
#if HAVE_CORBA
    orb_i_->shutdown();
    delete orb_i_;
#endif
}

#if HAVE_CORBA
AcquirePlugin::AcquirePlugin() : orb_i_( new orb_i() )
#else
AcquirePlugin::AcquirePlugin() 
#endif
{
}


bool
AcquirePlugin::initialize(const QStringList &arguments, QString *error_message)
{
    Q_UNUSED(arguments);
    Q_UNUSED(error_message);

    adportable::core::debug_core::instance()->hook( adlog::logging_handler::log );

    Core::Context context( (Core::Id( "Acquire.MainView" )), (Core::Id( Core::Constants::C_NAVIGATION_PANE )) );

    if ( AcquireMode * mode = new AcquireMode(this) ) {
        mode->setContext( context );
        
        if ( auto mainWindow = MainWindow::instance() ) {

            mainWindow->activateWindow();
            mainWindow->createActions();
            
            mode->setWidget( mainWindow->createContents( mode ) );
            
            addAutoReleasedObject(mode);
            mainWindow->setSimpleDockWidgetArrangement();
        }
    }

#if HAVE_CORBA
    auto qbroker = new QBroker();
    connect( qbroker, &QBroker::initialized, this, &AcquirePlugin::handle_broker_initialized );
    addObject( qbroker );
#endif

    if ( auto iAcquire = document::instance()->iAcquire() ) {
        addObject( iAcquire );
    }

    if ( auto iExtension = document::instance()->masterController() ) {
        addObject( iExtension );
        connect( iExtension, &adextension::iController::connected, MainWindow::instance(), &MainWindow::iControllerConnected );
    }

    return true;
}

void
AcquirePlugin::extensionsInitialized()
{
    if ( auto mainWindow = MainWindow::instance() ) {

        mainWindow->OnInitialUpdate();
        document::instance()->initialSetup();
        mainWindow->setControlMethod( *document::instance()->controlMethod() );
        mainWindow->setSampleRun( *document::instance()->sampleRun() );

        // gather and initialize control method,time events
        mainWindow->handleControlMethod();
    }
}

void
AcquirePlugin::handle_broker_initialized()
{
#if HAVE_CORBA
    if ( orb_i_ ) {
        orb_i_->initialize();
    }
#endif
}

ExtensionSystem::IPlugin::ShutdownFlag
AcquirePlugin::aboutToShutdown()
{
    document::instance()->actionDisconnect();

    if ( auto iAcquire = document::instance()->iAcquire() )
        removeObject( iAcquire );

#if HAVE_CORBA

    auto iBroker = ExtensionSystem::PluginManager::instance()->getObject< adextension::iBroker >();
    removeObject( iBroker );

#endif

    if ( auto mainWindow = MainWindow::instance() ) {

        document::instance()->finalClose( mainWindow );
        mainWindow->OnFinalClose();
    }

	return SynchronousShutdown;
}

void
AcquirePlugin::handle_shutdown()
{
    try { 
        MainWindow::instance()->handle_shutdown();
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
        assert( 0 );        
    }
}

void
AcquirePlugin::handle_debug_print( unsigned long priority, unsigned long category, QString text )
{
    try {
        MainWindow::instance()->handle_debug_print( priority, category, text );
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
        assert( 0 );        
    }
}

void
AcquirePlugin::handle_monitor_selected(int)
{
}

void
AcquirePlugin::handle_monitor_activated(int)
{
}

void
AcquirePlugin::handleSelected( const QPointF& pt )
{
	selectRange( pt.x(), pt.x(), pt.y(), pt.y() );
}

void
AcquirePlugin::handleSelected( const QRectF& rc )
{
	selectRange( rc.x(), rc.x() + rc.width(), rc.y(), rc.y() + rc.height() );
}

void
AcquirePlugin::selectRange( double x1, double x2, double y1, double y2 )
{
    (void)y1; (void)y2;
#if 0
    SignalObserver::Observers_var siblings = orb_i_->observer_->getSiblings();
    CORBA::ULong nsize = siblings->length();

    for ( CORBA::ULong i = 0; i < nsize; ++i ) {
        SignalObserver::Description_var desc = siblings[i]->getDescription();
        
        if ( desc->trace_method == SignalObserver::eTRACE_SPECTRA 
             && desc->spectrometer == SignalObserver::eMassSpectrometer ) {

            SignalObserver::Observer_var tgt = SignalObserver::Observer::_duplicate( siblings[i] );

            if ( pImpl_ && ! CORBA::is_nil( pImpl_->brokerSession_ ) ) {
				boost::filesystem::path path( adportable::profile::user_data_dir<char>() );
				path /= "data";
				path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
				if ( ! boost::filesystem::exists( path ) ) {
					boost::system::error_code ec;
					boost::filesystem::create_directories( path, ec );
				}
				path /= "acquire.adfs";
				
                try {
					pImpl_->brokerSession_->coaddSpectrum( path.string().c_str() /* L"acquire" */, tgt, x1, x2 );
                } catch ( std::exception& ex ) {
                    QMessageBox::critical( 0, "acquireplugin::handleRButtonRange", ex.what() );
                }
            }
        }
    }
#endif
}

Q_EXPORT_PLUGIN( AcquirePlugin )


///////////////////

