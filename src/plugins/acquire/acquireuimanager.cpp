// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "acquireuimanager.hpp"
#include "acquireactions.hpp"
#include <adplugin/ifactory.hpp>

#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>

#include <adwplot/dataplot.hpp>
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
#include <QMessageBox>
#include <QTabBar>

#include <qtwrapper/qstring.hpp>
#include <adplugin/adplugin.hpp>
#include <adplugin/constants.hpp>
#include <adplugin/imonitor.hpp>
#include <adplugin/lifecycleaccessor.hpp>
#include <adportable/configuration.hpp>
#include <adportable/string.hpp>
#include <adportable/debug.hpp>
#include <adinterface/eventlog_helper.hpp>
#include <acewrapper/timeval.hpp>

namespace Acquire { 
    namespace internal {
        
        struct AcquireUIManagerData : boost::noncopyable {
            AcquireUIManagerData() : mainWindow_(0) {}
            
            Utils::FancyMainWindow* mainWindow_;
            
            std::vector< QDockWidget * > dockWidgetVec_;
        };
        
    }
}

using namespace Acquire;
using namespace Acquire::internal;

AcquireUIManager::~AcquireUIManager()
{
    delete d_;
}

AcquireUIManager::AcquireUIManager(QObject *parent) : QObject(parent)
						    , d_(0)
{
    d_ = new AcquireUIManagerData();
}

QMainWindow *
AcquireUIManager::mainWindow() const
{
    return d_->mainWindow_;
}

void
AcquireUIManager::init()
{
    if ( ! d_ )
        return;
    
    Acquire::internal::AcquireUIManagerData& m = *d_;
    
    QDir dir = QCoreApplication::instance()->applicationDirPath();
    dir.cdUp();
    std::wstring apppath = qtwrapper::wstring::copy( dir.path() );
    // dir.cd( adpluginDirectory );
    std::wstring configFile = adplugin::orbLoader::config_fullpath( apppath, L"/MS-Cheminformatics/acquire.config.xml" );
    
    const wchar_t * query = L"/AcquireConfiguration/Configuration";
    
    adportable::Configuration config;
    adplugin::manager::instance()->loadConfig( config, configFile, query );
    
    m.mainWindow_ = new Utils::FancyMainWindow;
    
    if ( d_ && m.mainWindow_ ) {
        
        m.mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
        m.mainWindow_->setDocumentMode( true );
        
        const adportable::Configuration * pTab = adportable::Configuration::find( config, L"monitor_tab" );
        if ( pTab ) {
            using namespace adportable;
            using namespace adplugin;
            
            // std::wstring loadpath = qtwrapper::wstring( dir.path() );
            // tab pages
            for ( Configuration::vector_type::const_iterator it = pTab->begin(); it != pTab->end(); ++it ) {

                const std::wstring name = it->name();
                // const std::wstring& component = it->attribute( L"component" );
                
                if ( it->isPlugin() ) {
                    QWidget * pWidget = manager::widget_factory( *it, apppath.c_str(), 0 );

                    if ( pWidget ) {
						bool res = false;
						if ( it->interface() == L"adplugin::ui::iLog" ) {
							res = connect( this, SIGNAL( signal_eventLog( QString ) ), pWidget, SLOT( handle_eventLog( QString ) ) );
							emit signal_eventLog( "Hello -- this is acquire plugin" );
						}

                        pWidget->setWindowTitle( qtwrapper::qstring( it->title() ) );
                        QDockWidget * dock = m.mainWindow_->addDockForWidget( pWidget );
                        m.dockWidgetVec_.push_back( dock );
                    }

                }

            }

        }            
    }
}

void
AcquireUIManager::OnInitialUpdate()
{
    Acquire::internal::AcquireUIManagerData& m = *d_;

    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
		adplugin::LifeCycleAccessor accessor( dockWidget->widget() );
		adplugin::LifeCycle * pLifeCycle = accessor.get();
		if ( pLifeCycle )
			pLifeCycle->OnInitialUpdate();
    }
}

void
AcquireUIManager::OnFinalClose()
{
    QList< QDockWidget *> dockWidgets = d_->mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QObjectList list = dockWidget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycle * pLifeCycle = dynamic_cast<adplugin::LifeCycle *>( obj );
            if ( pLifeCycle )
                pLifeCycle->OnFinalClose();
        }
    }
}

class setTrackingEnabled {
    Utils::FancyMainWindow& w_;
public:
    setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) {
        w_.setTrackingEnabled( false );
    }
    ~setTrackingEnabled() {
        w_.setTrackingEnabled( true );
    }
};

void
AcquireUIManager::setSimpleDockWidgetArrangement()
{
    AcquireUIManagerData& m = *d_;
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
    
    // make dockwdigets into a tab
    for ( unsigned int i = 1; i < m.dockWidgetVec_.size(); ++i )
		m.mainWindow_->tabifyDockWidget( m.dockWidgetVec_[0], m.dockWidgetVec_[i] );

    QList< QTabBar * > tabBars = m.mainWindow_->findChildren< QTabBar * >();
    foreach( QTabBar * tabBar, tabBars ) 
        tabBar->setCurrentIndex( 0 );
}

void
AcquireUIManager::handle_message( unsigned long msg, unsigned long value )
{
   ACE_UNUSED_ARG(msg);
   ACE_UNUSED_ARG(value);
	// this is debugging purpose only, 
	// wired from AcquirePlugin::handle_message
}

void
AcquireUIManager::eventLog( const QString& text )
{
	emit signal_eventLog( text );
}

void
AcquireUIManager::handle_eventLog( const ::EventLog::LogMessage& log )
{
    using namespace adinterface::EventLog;
    std::wstring text = LogMessageHelper::toString( log );
	std::string date = acewrapper::to_string( log.tv.sec, log.tv.usec ) + "\t";
	// adportable::debug() << date << text;
	QString qtext = date.c_str();
	qtext += qtwrapper::qstring::copy( text );

    emit signal_eventLog( qtext );
}

void
AcquireUIManager::handle_shutdown()
{
}

void
AcquireUIManager::handle_debug_print( unsigned long priority, unsigned long category, QString text )
{
    Q_UNUSED( priority );
    Q_UNUSED( category );
    emit signal_eventLog( text );
}

