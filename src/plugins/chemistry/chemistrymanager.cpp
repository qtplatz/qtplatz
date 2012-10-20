/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "chemistrymanager.hpp"
#include <utils/styledbar.h>
#include <utils/fancymainwindow.h>
#include <adplugin/manager.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/lifecycle.hpp>
#include <qtwrapper/qstring.hpp>
#include <QDockWidget>
#include <QTabBar>

namespace Chemistry { namespace Internal {

    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
	public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };
} // Internal
}

using namespace Chemistry::Internal;

ChemistryManager::ChemistryManager( QObject * parent ) : QObject( parent )
{
}

QMainWindow *
ChemistryManager::mainWindow() const
{
	return mainWindow_.get();
}

void
ChemistryManager::init()
{
    mainWindow_.reset( new Utils::FancyMainWindow );
    if ( mainWindow_ ) {
		mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
        mainWindow_->setDocumentMode( true );
	}
}

void
ChemistryManager::OnInitialUpdate()
{
    QList< QDockWidget *> dockWidgets = mainWindow_->dockWidgets();
    
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QWidget * obj = dockWidget->widget();
		adplugin::LifeCycleAccessor accessor( obj );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle ) {
			pLifeCycle->OnInitialUpdate();
			connect( obj, SIGNAL( onMethodApply( adcontrols::ProcessMethod& ) )
                     , this, SLOT( onMethodApply( adcontrols::ProcessMethod& ) ), Qt::DirectConnection );
		}
    }
}

void
ChemistryManager::OnFinalClose()
{
}

void
ChemistryManager::setSimpleDockWidgetArrangement()
{
	setTrackingEnabled lock( *mainWindow_ );
  
    QList< QDockWidget *> dockWidgets = mainWindow_->dockWidgets();
  
	foreach ( QDockWidget * dockWidget, dockWidgets ) {
        dockWidget->setFloating( false );
		mainWindow_->removeDockWidget( dockWidget );
    }

    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        mainWindow_->addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->show();
    }

    for ( int i = 1; i < dockWidgets.size(); ++i )
		mainWindow_->tabifyDockWidget( dockWidgets[0], dockWidgets[i] );

	QList< QTabBar * > tabBars = mainWindow_->findChildren< QTabBar * >();
    foreach( QTabBar * tabBar, tabBars ) 
		tabBar->setCurrentIndex( 0 );
}
