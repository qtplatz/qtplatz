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
#include <adextension/icontroller.hpp>
#include <adextension/isequence.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adplugin/constants.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycleaccessor.hpp>
#include <adportable/configuration.hpp>
#include <adportable/string.hpp>
#include <adlog/logger.hpp>
#include <adwidgets/controlmethodwidget.hpp>
#include <adwidgets/samplerunwidget.hpp>
#include <qtwrapper/qstring.hpp>
#include <extensionsystem/pluginmanager.h>

#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>

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

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , cmEditor_( new adwidgets::ControlMethodWidget )
                                        , runEditor_( new adwidgets::SampleRunWidget )
{
    connect( cmEditor_, &adwidgets::ControlMethodWidget::onImportInitialCondition, this, &MainWindow::handleControlMethod );
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

    createDockWidget( runEditor_, "Sample Run",     "SampleRunWidget" ); // this must be first

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

    // and this must be very last.
    createDockWidget( cmEditor_, "Control Method", "ControlMethodWidget" );

	setSimpleDockWidgetArrangement();

    connect( cmEditor_, &adwidgets::ControlMethodWidget::onCurrentChanged, this, [this] ( QWidget * w ){ w->parentWidget()->raise(); } );

    for ( auto iController: ExtensionSystem::PluginManager::instance()->getObjects< adextension::iController >() ) {
        connect( iController, &adextension::iController::onControlMethodChanged, this, &MainWindow::handleControlMethod );
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

