//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "servantuimanager.hpp"

#include <QDockWidget>
#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QTextEdit>
#include <QPluginLoader>
#include <QLibrary>
#include <QtCore>
#include <QUrl>

#include <qtwrapper/qstring.hpp>
// #include <xmlwrapper/xmldom.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/imonitor.hpp>
#include <adportable/configuration.hpp>

using namespace servant;
using namespace servant::internal;

ServantUIManager::~ServantUIManager()
{
}

ServantUIManager::ServantUIManager(QObject *parent) :  QObject(parent)
{
}


QMainWindow *
ServantUIManager::mainWindow() const
{
	// mainWindow_;
	return 0;
}

void
ServantUIManager::init()
{
    QDir dir = QCoreApplication::instance()->applicationDirPath();
    dir.cdUp();
    dir.cd( "lib/qtPlatz/plugins/MS-Cheminformatics" );

    QString configFile = dir.path() + "/servant.config.xml";
	const wchar_t * query = L"/ServantConfiguration/Configuration";

    adportable::Configuration config;
    adplugin::manager::instance()->loadConfig( config, qtwrapper::wstring( configFile ), query );

	mainWindow_ = new Utils::FancyMainWindow;

	if ( mainWindow_ ) {
        mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
        mainWindow_->setDocumentMode( true );
    }
}

void
ServantUIManager::setSimpleDockWidgetArrangement()
{
/*
    ServantUIManagerData& m = *d_;
    setTrackingEnabled lock( *m.mainWindow_ );
    
    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
    
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        dockWidget->setFloating( false );
        m.mainWindow_->removeDockWidget( dockWidget );
    }
    
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        m.mainWindow_->addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->show();
    }
    
    for ( unsigned int i = 2; i < m.dockWidgetVec_.size(); ++i )
        m.mainWindow_->tabifyDockWidget( m.dockWidgetVec_[1], m.dockWidgetVec_[i] );
*/
}
