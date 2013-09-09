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

#include "mainwindow.hpp"
#include <adplugin/widget_factory.hpp>
#include <adwplot/dataplot.hpp>
#include <adplugin/constants.hpp>
#include <adplugin/lifecycle.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adportable/configuration.hpp>
#include <adportable/string.hpp>
#include <adportable/debug.hpp>
// #include <adinterface/eventlog_helper.hpp>
#include <acewrapper/timeval.hpp>
#include <qtwrapper/qstring.hpp>

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
{
}


void
MainWindow::init( const adportable::Configuration& config )
{
    // Acquire::internal::MainWindowData& m = *d_;
    
	setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    setDocumentMode( true );

    const adportable::Configuration * pTab = adportable::Configuration::find( config, "monitor_tab" );
    if ( pTab ) {
        using namespace adportable;
        using namespace adplugin;
            
        for ( auto node: *pTab ) {
			
            const std::string name = node.name();
                
			QWidget * pWidget = adplugin::widget_factory::create( node.component_interface().c_str(), 0, 0 );

            if ( pWidget ) {
                if ( node.component_interface() == "adplugin::ui::iLog" ) {
                    connect( this, SIGNAL( signal_eventLog( QString ) ), pWidget, SLOT( handle_eventLog( QString ) ) );
                    emit signal_eventLog( "Hello -- this is acquire plugin" );
                }

                pWidget->setWindowTitle( qtwrapper::qstring( node.title() ) );
                QDockWidget * dock = addDockForWidget( pWidget );
                dockWidgetVec_.push_back( dock );
            }


        }

    }            
}

void
MainWindow::OnInitialUpdate()
{
	QList< QDockWidget *> dockWidgets = this->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
		adplugin::LifeCycleAccessor accessor( dockWidget->widget() );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle )
			pLifeCycle->OnInitialUpdate();
    }
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

void
MainWindow::addMonitorWidget( QWidget * widget, const QString& title )
{
    if ( widget ) {
        widget->setWindowTitle( title );
        QDockWidget * dock = addDockForWidget( widget );
        dockWidgetVec_.push_back( dock );
    }
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
    
    QList< QDockWidget *> dockWidgets = this->dockWidgets();

    for ( auto widget: dockWidgets ) {
		widget->setFloating( false );
		removeDockWidget( widget );
	}
    
    for ( auto widget: dockWidgets ) {
        addDockWidget( Qt::BottomDockWidgetArea, widget );
        widget->show();
    }
    
    // make dockwdigets into a tab
	std::for_each( dockWidgets.begin() + 1, dockWidgets.end(), [&]( QDockWidget * widget ) {
		tabifyDockWidget( dockWidgets[0], widget );
	});

    QList< QTabBar * > tabBars = findChildren< QTabBar * >();
	std::for_each( tabBars.begin(), tabBars.end(), []( QTabBar * tabBar ){ tabBar->setCurrentIndex( 0 ); }); 
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

