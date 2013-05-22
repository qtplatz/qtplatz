/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "chemspidermanager.hpp"
#include <utils/styledbar.h>

#include <utils/fancymainwindow.h>
#include <adplugin/manager.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adplugin/lifecycle.hpp>
#include <adportable/configuration.hpp>
#include <qtwrapper/qstring.hpp>
#include <QDockWidget>
#include <QTabBar>

namespace ChemSpider { namespace Internal {

    class setTrackingEnabled {
        Utils::FancyMainWindow& w_;
	public:
        setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
        ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
    };
} // Internal
}

using namespace ChemSpider::Internal;

ChemSpiderManager::ChemSpiderManager(QObject *parent) : QObject(parent)
{
}

QMainWindow *
ChemSpiderManager::mainWindow() const
{
	return mainWindow_.get();
}

void
ChemSpiderManager::init( const adportable::Configuration& config, const std::wstring& apppath )
{
	mainWindow_.reset( new Utils::FancyMainWindow );
    if ( mainWindow_ ) {
        mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
        mainWindow_->setDocumentMode( true );
    }

	const adportable::Configuration * pTab = adportable::Configuration::find( config, L"MolFiles" );
    if ( pTab ) {
        using namespace adportable;
        using namespace adplugin;
            
        for ( Configuration::vector_type::const_iterator it = pTab->begin(); it != pTab->end(); ++it ) {
            const std::wstring name = it->name();
            if ( it->isPlugin() ) {
                QWidget * pWidget = manager::widget_factory( *it, apppath.c_str(), 0 );
                if ( pWidget ) {
                    // query process method
                    connect( this, SIGNAL( signalGetProcessMethod( adcontrols::ProcessMethod& ) )
                             , pWidget, SLOT( getContents( adcontrols::ProcessMethod& ) ), Qt::DirectConnection );
                    pWidget->setMinimumHeight( 80 );
                    pWidget->setWindowTitle( qtwrapper::qstring( it->title() ) );
					mainWindow_->addDockForWidget( pWidget );
                }
            }
        }
    }       
}


void
ChemSpiderManager::OnInitialUpdate()
{
    QList< QDockWidget *> dockWidgets = mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QWidget * obj = dockWidget->widget();
		adplugin::LifeCycleAccessor accessor( obj );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle ) {
			pLifeCycle->OnInitialUpdate();
			bool res = connect( obj, SIGNAL( apply( adcontrols::ProcessMethod& ) ), this, SLOT( handleApply( adcontrols::ProcessMethod& ) ), Qt::DirectConnection );
			assert( res );
		}
    }
}

void
ChemSpiderManager::OnFinalClose()
{
}

void
ChemSpiderManager::setSimpleDockWidgetArrangement()
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

void
ChemSpiderManager::handleApply( adcontrols::ProcessMethod& )
{
	// not implemented
}
