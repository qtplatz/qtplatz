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
#include <adextension/isequence.hpp>
#include <adextension/ieditorfactory.hpp>
#include <adplugin/widget_factory.hpp>
#include <adplugin/constants.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adportable/configuration.hpp>
#include <adportable/string.hpp>
#include <adlog/logger.hpp>
#include <adwidgets/controlmethodwidget.hpp>
#include <adwidgets/controlmethodcontainer.hpp>
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
using namespace acquire::internal;

MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QWidget *parent) : Utils::FancyMainWindow(parent)
                                        , editor_(0)
{
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
    editor_ = new adwidgets::ControlMethodWidget;
    editor_->OnInitialUpdate();

    auto visitables = ExtensionSystem::PluginManager::instance()->getObjects< adextension::iSequence >();

	for ( auto v: visitables ) {

        for ( size_t i = 0; i < v->size(); ++i ) {

            adextension::iEditorFactory& factory = (*v)[ i ];
            if ( factory.method_type() == adextension::iEditorFactory::CONTROL_METHOD ) {

                if ( auto widget = factory.createEditor( 0 ) ) {
                    widget->setObjectName( factory.title() );
                    createDockWidget( widget, factory.title(), "ControlMethod" );
                    editor_->addEditor( widget ); // will call OnInitialUpdate
                }
                
            }

        }
    }

    connect( editor_, &adwidgets::ControlMethodWidget::onCurrentChanged, this, [this] ( QWidget * w ){ w->parentWidget()->raise(); } );

    createDockWidget( editor_, "Control Method", "ControlMethodWidget" );

	setSimpleDockWidgetArrangement();
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
MainWindow::setControlMethod( const adcontrols::ControlMethod& m )
{
    editor_->setControlMethod( m );
}

void
MainWindow::getControlMethod( adcontrols::ControlMethod& m )
{
    editor_->getControlMethod( m );
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

