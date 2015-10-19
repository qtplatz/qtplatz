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

#include "mainwindow.hpp"
#include "document.hpp"
#include "masterobserver.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adextension/icontroller.hpp>
#include <adextension/isequence.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adicontroller/instrument.hpp>
#include <adicontroller/receiver.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adplugin/constants.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycleaccessor.hpp>
#include <adportable/configuration.hpp>
#include <adportable/debug.hpp>
#include <adportable/string.hpp>
#include <adportable/utf.hpp>
#include <adlog/logger.hpp>
#include <adwidgets/controlmethodwidget.hpp>
#include <adwidgets/insttreeview.hpp>
#include <adwidgets/samplerunwidget.hpp>
#include <qtwrapper/qstring.hpp>
#include <extensionsystem/pluginmanager.h>

#include <boost/variant.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QTextEdit>
#include <QPluginLoader>
#include <QLibrary>
#include <QtCore>
#include <QUrl>
#include <QMessageBox>
#include <QTabBar>

using namespace acquire;

MainWindow * MainWindow::instance_; 

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , cmEditor_( new adwidgets::ControlMethodWidget )
                                        , runEditor_( new adwidgets::SampleRunWidget )
{
    instance_ = this;
    connect( cmEditor_, &adwidgets::ControlMethodWidget::onImportInitialCondition, this, &MainWindow::handleControlMethod );
}

size_t 
MainWindow::findInstControllers( std::vector< std::shared_ptr< adextension::iController > >& vec ) const
{
    for ( auto v : ExtensionSystem::PluginManager::getObjects< adextension::iController >() ) {
        ADDEBUG() << v->module_name().toStdString();
        try {
            vec.push_back( v->shared_from_this() );
        } catch ( std::bad_weak_ptr& ) {
#if defined _DEBUG || defined DEBUG
            ADDEBUG() << "adextension::iController does not inherit from shared_ptr";
#endif
            // ignore old iController implementation
        }
    }
    return vec.size();
}

MainWindow * 
MainWindow::instance()
{
    return instance_;
}

void
MainWindow::init( const adportable::Configuration& config )
{
	setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    setDocumentMode( true );
}

void
MainWindow::OnInitialUpdate()
{
    cmEditor_->OnInitialUpdate();
    runEditor_->OnInitialUpdate();

    createDockWidget( runEditor_, "Sample Run", "SampleRunWidget" ); // this must be first

    // then, series of individual control method widgets
    auto visitables = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSequence >();

	for ( auto v: visitables ) {

        for ( size_t i = 0; i < v->size(); ++i ) {

            const adextension::iEditorFactory& factory = ( *v )[ i ];
            if ( factory.method_type() == adextension::iEditorFactory::CONTROL_METHOD ) {

                if ( auto widget = factory.createEditor( 0 ) ) {
                    widget->setObjectName( factory.title() );
                    createDockWidget( widget, factory.title(), "ControlMethod" );
                    cmEditor_->addEditor( widget ); // will call OnInitialUpdate
                }
                
            }

        }
    }

    if ( auto tree = new adwidgets::InstTreeView() )
        createDockWidget( tree, "Status", "InstStatus" );

    // and this must be very last.
    createDockWidget( cmEditor_, "Control Method", "ControlMethodWidget" );

	setSimpleDockWidgetArrangement();

    connect( cmEditor_, &adwidgets::ControlMethodWidget::onCurrentChanged, this, [this] ( QWidget * w ){ w->parentWidget()->raise(); } );

    auto instTree = findChild< adwidgets::InstTreeView * >();

    for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
        connect( iController, &adextension::iController::onControlMethodChanged, this, &MainWindow::handleControlMethod );
        if ( instTree )
            instTree->addItem( iController->module_name(), iController->module_name(), false );
    }
}

void
MainWindow::OnFinalClose()
{
    QList< QDockWidget *> dockWidgets = this->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QObjectList list = dockWidget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycle * pLifeCycle = dynamic_cast<adplugin::LifeCycle *>( obj );
            if ( pLifeCycle )
                pLifeCycle->OnFinalClose();
        }
    }
}

QDockWidget *
MainWindow::createDockWidget( QWidget * widget, const QString& title, const QString& objname )
{
    if ( widget->windowTitle().isEmpty() ) // avoid QTC_CHECK warning on console
        widget->setWindowTitle( title );

    if ( widget->objectName().isEmpty() )
        widget->setObjectName( objname );

    QDockWidget * dockWidget = addDockForWidget( widget );
    dockWidget->setObjectName( widget->objectName() );
    if ( title.isEmpty() )
        dockWidget->setWindowTitle( widget->objectName() );
    else
        dockWidget->setWindowTitle( title );

    if ( !objname.isEmpty() )
        dockWidget->setObjectName( objname );        

    addDockWidget( Qt::BottomDockWidgetArea, dockWidget );

    return dockWidget;
}

class scopedSetTrackingEnabled {
    Utils::FancyMainWindow& w_;
public:
    scopedSetTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) {
        w_.setTrackingEnabled( false );
    }
    ~scopedSetTrackingEnabled() {
        w_.setTrackingEnabled( true );
    }
};

void
MainWindow::setSimpleDockWidgetArrangement()
{
    scopedSetTrackingEnabled lock( *this );
    
    auto widgets = dockWidgets();
    for ( auto widget: widgets ) {
		widget->setFloating( false );
		removeDockWidget( widget );
	}
    
    size_t nsize = widgets.size();
    size_t npos = 0;
    for ( auto widget: widgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
        if ( npos++ >= 1 && npos < nsize )
            tabifyDockWidget( widgets[0], widget );
    }

    if ( !widgets.isEmpty() )
        (*widgets.begin())->raise();
}

void
MainWindow::handle_message( unsigned long msg, unsigned long value )
{
   ACE_UNUSED_ARG(msg);
   ACE_UNUSED_ARG(value);
	// this is debugging purpose only, 
	// wired from AcquirePlugin::handle_message
}

void
MainWindow::eventLog( const QString& text )
{
	emit signal_eventLog( text );
}

void
MainWindow::setControlMethod( const adcontrols::ControlMethod::Method& m )
{
    cmEditor_->setControlMethod( m );
}

std::shared_ptr< adcontrols::ControlMethod::Method >
MainWindow::getControlMethod()
{
    auto mp = std::make_shared< adcontrols::ControlMethod::Method >();
    cmEditor_->getControlMethod( *mp );
    return mp;
}

void
MainWindow::getControlMethod( adcontrols::ControlMethod::Method& m )
{
    cmEditor_->getControlMethod( m );
}

void
MainWindow::setSampleRun( const adcontrols::SampleRun& m )
{
    runEditor_->setSampleRun( m );
}

bool
MainWindow::getSampleRun( adcontrols::SampleRun& m )
{
    runEditor_->getSampleRun( m );
    return true;
}


void
MainWindow::handle_shutdown()
{
}

void
MainWindow::handle_debug_print( unsigned long priority, unsigned long category, QString text )
{
    Q_UNUSED( priority );
    Q_UNUSED( category );
    emit signal_eventLog( text );
}

void
MainWindow::handleControlMethod()
{
    auto ptr = acquire::document::instance()->controlMethod();

    for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
        iController->preparing_for_run( *ptr );
    }
    if ( cmEditor_ )
        cmEditor_->setControlMethod( *ptr );
}

struct observer2ptree {

    void operator()( adicontroller::SignalObserver::Observer * observer, boost::property_tree::ptree& pt ) {

        const auto& desc = observer->description();

        pt.put( "observer.objid", observer->objid() );        
        pt.put( "observer.objtext", observer->objtext() );
        pt.put( "observer.desc.trace_id", desc.trace_id() );
        pt.put( "observer.desc.trace_display_name", adportable::utf::to_utf8( desc.trace_display_name() ) );
        pt.put( "observer.desc.trace_method", desc.trace_method() );

        int count( 0 );
        boost::property_tree::ptree vec;

        for ( auto sibling : observer->siblings() ) {
            boost::property_tree::ptree child;
            observer2ptree()( sibling.get(), child );
            vec.push_back( std::make_pair( "", child ) );
            ++count;
        }
        if ( count )
            pt.add_child( "siblings", vec );
    }
    
};

void
MainWindow::iControllerConnected( adextension::iController * inst )
{
    ADDEBUG() << "iControllerConnected( adextension::iController * inst )";

    if ( auto tree = findChild< adwidgets::InstTreeView * >() ) {

        tree->setChecked( inst->module_name(), true );

        boost::property_tree::ptree pt;
        if ( inst->module_name() == "Acquire" ) {
            observer2ptree()( document::instance()->masterObserver(), pt );
        } else {
            if ( auto session = inst->getInstrumentSession() ) {
                if ( auto observer = session->getObserver() ) 
                    observer2ptree()( observer, pt );
            }
        }
        std::ostringstream o;
        boost::property_tree::write_json( o, pt );
        tree->setObserverTree( inst->module_name(), QString::fromStdString( o.str() ) );
    }
}

void
MainWindow::iControllerMessage( adextension::iController * p, uint32_t msg, uint32_t value )
{
    ADDEBUG() << "iControllerMessage(" << p->module_name().toStdString() << ", msg=" << msg << ", value=" << value << ")";
    static const QString state_names[] = {
        "Nothing"
        , "Not Connected"             //= 0x00000001,  // no instrument := no driver software loaded
        , "Off"                      //= 0x00000002,  // software driver can be controled, but hardware is currently off
        , "Initializing"             //= 0x00000003,  // startup initializing (only at the begining after startup)
        , "StandBy"                  //= 0x00000004,  // instrument is stand by state
        , "PreparingForRun"          //= 0x00000005,  // preparing for next method (parameters being be set value)
        , "ReadyForRun"              //= 0x00000006,  // method is in initial state, ready to run (INIT RUN, MS HTV is ready)
        , "WaitingForContactClosure" //= 0x00000007,  //
        , "Running"                  //= 0x00000008,  // method is in progress
        , "Stop"                     //= 0x00000009,  // stop := detector is not monitoring, pump is off
    };

    if ( auto tree = findChild< adwidgets::InstTreeView * >() ) {
        if ( msg == adicontroller::Receiver::STATE_CHANGED && value < sizeof(state_names)/sizeof(state_names[0]) ) {
            tree->setInstState( p->module_name(), state_names[ value ] );
        }
    }
}

