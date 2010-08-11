//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "acquireuimanager.h"
#include "acquireactions.h"
#include <adplugin/ifactory.h>

#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>

#include <adwidgets/dataplot.h>
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

#include <qtwrapper/qstring.h>
#include <xmlwrapper/xmldom.h>
#include <adplugin/adplugin.h>
#include <adplugin/imonitor.h>
#include <adportable/configuration.h>

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
    dir.cd( "lib/qtPlatz/plugins/ScienceLiaison" );

    QString configFile = dir.path() + "/acquire.config.xml";

	const wchar_t * query = L"/AcquireConfiguration/Configuration";

    adportable::Configuration config;
	adplugin::manager::instance()->loadConfig( config, configFile, query );

    m.mainWindow_ = new Utils::FancyMainWindow;

    if ( d_ && m.mainWindow_ ) {

        m.mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
        m.mainWindow_->setDocumentMode( true );
        
        const adportable::Configuration * pTab = adportable::Configuration::find( config, L"instrument_monitor_tab" );
        if ( pTab ) {
            using namespace adportable;
            using namespace adplugin;

			std::wstring loadpath = qtwrapper::wstring( dir.path() );
            // tab pages
            for ( Configuration::vector_type::const_iterator it = pTab->begin(); it != pTab->end(); ++it ) {
                
                const std::wstring name = it->name();
				// const std::wstring& component = it->attribute( L"component" );

                if ( it->isPlugin() ) {
					QWidget * pWidget = manager::widget_factory( *it, loadpath.c_str(), 0 );
                    pWidget->setWindowTitle( qtwrapper::qstring( it->title() ) );
                    QDockWidget * dock = m.mainWindow_->addDockForWidget( pWidget );
                    m.dockWidgetVec_.push_back( dock );
                } else {
                    QWidget * pWidget = new QTextEdit( qtwrapper::qstring( it->title() ) );
                    pWidget->setWindowTitle( qtwrapper::qstring( it->title() ) );
                    QDockWidget * dock = m.mainWindow_->addDockForWidget( pWidget );
                    m.dockWidgetVec_.push_back( dock );
                }
            }
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

  for ( unsigned int i = 2; i < m.dockWidgetVec_.size(); ++i )
    m.mainWindow_->tabifyDockWidget( m.dockWidgetVec_[1], m.dockWidgetVec_[i] );

}
