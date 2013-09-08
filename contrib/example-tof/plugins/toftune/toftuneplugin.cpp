/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "toftuneplugin.hpp"
#include "receiver_i.hpp"
#include "toftuneconstants.hpp"
#include "toftunemode.hpp"
#include "isequenceimpl.hpp"
#include "mainwindow.hpp"
#include "sideframe.hpp"
#include <tofspectrometer/constants.hpp>
#include <tofinterface/method.hpp>
#include <adextension/isequence.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adwplot/chromatogramwidget.hpp>
#include <adwplot/spectrumwidget.hpp>
#include <adplugin/qobserverevents_i.hpp>
#include <adportable/serializer.hpp>
#include <adorbmgr/orbmgr.hpp>
#include <adplugin/adplugin.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/trace.hpp>
#include <adcontrols/traceaccessor.hpp>
#include <adinterface/eventlog_helper.hpp>
#include <adinterface/controlmethodhelper.hpp>
#include <qtwrapper/qstring.hpp>

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/modemanager.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>

#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>
#include <QHBoxLayout>
#include <QtCore/QtPlugin>
#include <QDebug>
#include <cstring>

using namespace toftune::Internal;

tofTunePlugin * tofTunePlugin::instance_ = 0;

tofTunePlugin *
tofTunePlugin::instance()
{
    return instance_;
}

tofTunePlugin::tofTunePlugin() : receiver_i_( new Receiver_i<tofTunePlugin>( * this ) )
                               , hasController_( false )
{
    traces_.push_back( std::shared_ptr< adcontrols::Trace > ( new adcontrols::Trace( 0 ) ) );
	instance_ = this;
}

tofTunePlugin::~tofTunePlugin()
{
    if ( mode_ )
        removeObject( mode_.get() );

    if ( iSequenceImpl_ )
        removeObject( iSequenceImpl_.get() );
}

bool
tofTunePlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorString)

	Core::ActionManager * am = Core::ICore::instance()->actionManager();
    QAction * action = new QAction( tr("TOF Action" ), this );
    QList< int > globalcontext;
    globalcontext << Core::Constants::C_GLOBAL_ID;
    Core::Command * cmd = am->registerAction( action, Constants::ACTION_ID, globalcontext );
    cmd->setDefaultKeySequence( QKeySequence( tr("Ctrl+Alt+Meta+A" ) ) );
    connect( action, SIGNAL( triggered() ), this, SLOT( triggerAction() ) );

    Core::ActionContainer * menu = am->createMenu( Constants::MENU_ID );
    menu->menu()->setTitle( tr("TOF" ) );
    menu->addAction( cmd );
    am->actionContainer( Core::Constants::M_TOOLS )->addMenu( menu );

    mode_.reset( new tofTuneMode( this ) );
    if ( ! mode_ )
        return false;

    do {
        mainWindow_.reset( new MainWindow(0) );
        if ( ! mainWindow_ )
            return false;
    } while ( 0 );

    iSequenceImpl_.reset( new iSequenceImpl );
    if ( iSequenceImpl_ && mainWindow_->editor_factories( *iSequenceImpl_ ) )
        addObject( iSequenceImpl_.get() );

/*
    QList< adextension::iSequence * > adapters =
        ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSequence >();
    for ( adextension::iSequence * s: adapters ) {
        EditorFactory factory;
        s->addEditorFactory( &factory );
    }
*/    
    Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
    mainWindow_->activateLayout();
    mainWindow_->createActions( this );
    QWidget * widget = mainWindow_->createContents( mode_.get() );
    mode_->setWidget( widget );
    addObject( mode_.get() );
	
    return true;

}

void
tofTunePlugin::extensionsInitialized()
{
    // Retrieve objects from the plugin manager's object pool
    // "In the extensionsInitialized method, a plugin can be sure that all
    //  plugins that depend on it are completely initialized."
    Core::ModeManager::instance()->activateMode( mode_->uniqueModeName() );
    mainWindow_->OnInitialUpdate();

	Broker::Manager_var mgr = adorbmgr::orbmgr::getBrokerManager();
    CORBA::ORB_var orb = adorbmgr::orbmgr::instance()->orb();
    
	if ( ! CORBA::is_nil( mgr.in() ) ) {
        brokerSession_ = mgr->getSession( L"TOF" );
        if ( ! CORBA::is_nil( brokerSession_ ) ) {
            brokerSession_->connect( "TOF", "nopassword", "tof/toftune", 0 );

            // servant iid "com.ms-cheminfo.qtplatz.instrument.session.tofservant" is defined in file
			// tofservant/tofmgr_i.cpp, class tofmgr_i is a BrokerClient instance that register iid/obj pair
			// to Broker as return on setBrokerManager() method call.

			CORBA::Object_var obj = mgr->find_object( "com.ms-cheminfo.qtplatz.instrument.session.tofservant" );
            receiver_i_->session_ = TOF::Session::_duplicate( TOF::Session::_narrow( obj ) );
            if ( ! CORBA::is_nil( receiver_i_->session_ ) ) {
				hasController_ = true;
            } else {
                adportable::debug( __FILE__, __LINE__ ) << "Can't find tofcontroller::session";
			}

        }
    }
	if ( hasController_ ) {
		receiver_i_->session_->connect( receiver_i_->_this(), "tofTunePlugin::extensionsInitialized" );
        ControlMethod::Method_var method = receiver_i_->session_->getControlMethod();
        mainWindow_->setMethod( *method );

		connect( this, SIGNAL( OnMessage( unsigned long, unsigned long ) ), this, SLOT( HandleMessage( unsigned long, unsigned long ) ) );
        connect( this, SIGNAL( OnLog(QString, QString) ), this, SLOT( HandleLog( QString, QString ) ) );

		signalObserver_ = SignalObserver::Observer::_duplicate( receiver_i_->session_->getObserver() );
  
        if ( ! CORBA::is_nil( signalObserver_.in() ) ) {
			signalObserverEvents_i_.reset( new adplugin::QObserverEvents_i( signalObserver_, "tofTunePlugin::extensionsInitialized" ) );
			connect( signalObserverEvents_i_.get(), SIGNAL( signal_UpdateData( unsigned long, long ) ), this, SLOT( HandleUpdateData( unsigned long, long ) ) );
		}
	}

}

ExtensionSystem::IPlugin::ShutdownFlag
tofTunePlugin::aboutToShutdown()
{
	// PortableServer::POA_var poa = adplugin::ORBManager::instance()->poa();
	try {
        if ( ! CORBA::is_nil( signalObserver_.in() ) && signalObserverEvents_i_ ) {
             disconnect( signalObserverEvents_i_.get(), SIGNAL( signal_UpdateData( unsigned long, long ) ), this, SLOT( HandleUpdateData( unsigned long, long ) ) );
			 signalObserverEvents_i_->disconnect();
			 adorbmgr::orbmgr::instance()->deactivate( signalObserverEvents_i_->_this() );
			 signalObserver_->_remove_ref();
		}
	} catch ( CORBA::Exception& ex ) {
		adportable::debug( __FILE__, __LINE__ ) << ex._info().c_str();
	}

	try {
		if ( ! CORBA::is_nil( receiver_i_->session_ ) ) {
           receiver_i_->session_->disconnect( receiver_i_->_this() );
		   adorbmgr::orbmgr::instance()->deactivate( receiver_i_->_this() );
		   receiver_i_->session_->_remove_ref();
		}
	} catch ( CORBA::Exception& ex ) {
		adportable::debug( __FILE__, __LINE__ ) << ex._info().c_str(); 
	}

	try {
		if ( ! CORBA::is_nil( brokerSession_ ) ) 
			brokerSession_->disconnect( 0 );
	} catch ( CORBA::Exception& ex ) {
		adportable::debug( __FILE__, __LINE__ ) << ex._info().c_str(); 
	}

    return SynchronousShutdown;
}

void
tofTunePlugin::triggerAction()
{
    QMessageBox::information(Core::ICore::instance()->mainWindow(),
                             tr("Action triggered"),
                             tr("This is an action from toftune."));
}


void
tofTunePlugin::onMessage( unsigned long msg, unsigned long value )
{
	emit OnMessage( msg, value );
}

void
tofTunePlugin::onLog( const EventLog::LogMessage& log )
{
	std::wstring text = adinterface::EventLog::LogMessageHelper::toString( log );
	QString key = qtwrapper::qstring::copy( log.srcId.in() );
    QString qtext = qtwrapper::qstring::copy( text );
    emit OnLog( key, qtext );
}

void
tofTunePlugin::onPrint( long pri, long cat, const char * text )
{
	qDebug() << "tofTunePlugin::onPrint " << pri << " cat: " << cat << "\t" << text;
}

void
tofTunePlugin::HandleMessage( unsigned long msg, unsigned long value )
{
	qDebug() << "tofTunePlugin::HandleMessagee(" << msg << ", " << value << ")";
	if ( msg == Receiver::STATE_CHANGED ) {
		Instrument::eInstStatus status = static_cast< Instrument::eInstStatus >( receiver_i_->session_->get_status() );
        (void)status;
	}
}

void
tofTunePlugin::HandleLog( QString key, QString text )
{
	qDebug() << "tofTunePlugin::HandleLog(" << key << ", " << text << ")";
}

void
tofTunePlugin::HandleUpdateData( unsigned long objId, long pos )
{
	const wchar_t * dname = tofspectrometer::constants::dataInterpreter::spectrometer::name();
    const adcontrols::MassSpectrometer& spectrometer = adcontrols::MassSpectrometer::get( dname );
    const adcontrols::DataInterpreter& interpreter = spectrometer.getDataInterpreter();

    try { 
        if ( objId == signalObserver_->objId() ) {
            SignalObserver::DataReadBuffer_var rb;
            if ( signalObserver_->readData( pos, rb ) ) {
                adcontrols::MassSpectrum ms;
                size_t idData = 0;
				if ( interpreter.translate( ms, reinterpret_cast< const char * >(rb->xdata.get_buffer()), rb->xdata.length()
					, reinterpret_cast< const char * >(rb->xmeta.get_buffer()), rb->xmeta.length(), spectrometer, idData ) )
                    mainWindow_->setData( ms );
            }
            return;
        }
    } catch ( CORBA::Exception& ) {
        assert( 0 );
    }

    try {
        SignalObserver::Observer_var so = signalObserver_->findObserver( objId, true );
        if ( CORBA::is_nil( so.in() ) )
            return;

        SignalObserver::DataReadBuffer_var rb;

        if ( so->readData( pos, rb ) ) {

            SignalObserver::Description_var desc = so->getDescription();
            CORBA::WString_var clsid = so->dataInterpreterClsid();

            // std::wcout << "handle data: " << clsid.in() << "\tpos: " << pos << std::endl;
            if ( desc->trace_method == SignalObserver::eTRACE_TRACE ) {
                std::wstring traceId = static_cast<const CORBA::WChar *>( desc->trace_id );

                adcontrols::TraceAccessor accessor;
				if ( interpreter.translate( accessor
					, reinterpret_cast< const char * >( rb->xdata.get_buffer() ), rb->xdata.length()
					, reinterpret_cast< const char * >( rb->xmeta.get_buffer() ), rb->xmeta.length(), rb->events ) ) {
						for ( size_t fcn = 0; fcn < accessor.nfcn() && fcn < traces_.size(); ++fcn ) {
							adcontrols::Trace& trace = *traces_[ fcn ];
							accessor >> trace;
							mainWindow_->setData( trace, traceId );
						}
                }
            }
        }
    } catch ( CORBA::Exception& ex ) {
        std::cout << ex._info() << std::endl;
        assert( 0 );
    }
}

void
tofTunePlugin::setMethod( const tof::ControlMethod& m, const std::string& hint )
{
    if ( ! CORBA::is_nil( receiver_i_ ) && ! CORBA::is_nil( receiver_i_->session_ ) ) {
		std::string device;
		
		TOF::octet_array oa;
		if ( adportable::serializer< tof::ControlMethod >::serialize( m, device ) ) {
			oa.length( device.size() );
			std::copy( device.begin(), device.end(), oa.get_buffer() );
		}
		receiver_i_->session_->setControlMethod( oa, hint.c_str() );
    }
}

void
tofTunePlugin::actionConnect()
{
    if ( ! CORBA::is_nil( receiver_i_ ) && ! CORBA::is_nil( receiver_i_->session_ ) )
        receiver_i_->session_->initialize();
}

Q_EXPORT_PLUGIN2(toftune, tofTunePlugin)

