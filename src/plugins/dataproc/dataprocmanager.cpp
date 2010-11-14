//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "dataprocmanager.h"
#include <adportable/Configuration.h>
#include <adcontrols/datafilebroker.h>
#include <adplugin/adplugin.h>
#include <adplugin/lifecycle.h>
#include <qtwrapper/qstring.h>
#include <boost/noncopyable.hpp>

#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
#include <QtCore/QHash>
#include <QString>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QTextEdit>

#include <vector>

using namespace dataproc::internal;

namespace dataproc {
  namespace internal {
    class DataprocManagerImpl : boost::noncopyable {
    public:
      ~DataprocManagerImpl();
      DataprocManagerImpl();

      Utils::FancyMainWindow * mainWindow_;
      void init();

    public:
      std::vector< QDockWidget * > dockWidgetVec_;

    };
  }
}

DataprocManager::DataprocManager(QObject *parent) :
    QObject(parent)
    , pImpl_( new DataprocManagerImpl() )
{
}

QMainWindow *
DataprocManager::mainWindow() const
{
    return pImpl_->mainWindow_;
}

void
DataprocManager::init( const adportable::Configuration& config, const std::wstring& apppath )
{
    pImpl_->init();

    DataprocManagerImpl& m = *pImpl_;

    const adportable::Configuration * pTab = adportable::Configuration::find( config, L"ProcessMethodEditors" );
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
                    //pWidget->setWindowTitle( tr( qtwrapper::qstring::copy(it->name())) );
                    //connect( this, SIGNAL( signal_eventLog( QString ) ), pWidget, SLOT( handle_eventLog( QString ) ) );
                    pWidget->setWindowTitle( qtwrapper::qstring( it->title() ) );
                    QDockWidget * dock = m.mainWindow_->addDockForWidget( pWidget );
                    m.dockWidgetVec_.push_back( dock );

                } else {
                    QWidget * edit = new QTextEdit( "Edit" );
                    edit->setWindowTitle( qtwrapper::qstring( it->title() ) );
                    QDockWidget * dock = m.mainWindow_->addDockForWidget( edit );
                    m.dockWidgetVec_.push_back( dock );
                }
            }
        }
    }       

    ////
    const adportable::Configuration * provider = adportable::Configuration::find( config, L"dataproviders" );
    if ( provider ) {
        using namespace adportable;
        using namespace adplugin;

        for ( Configuration::vector_type::const_iterator it = provider->begin(); it != provider->end(); ++it ) {
            const std::wstring name = apppath + it->module().library_filename();
            adcontrols::datafileBroker::register_library( name );
        }
    }
    
}

void
DataprocManager::OnInitialUpdate()
{
    DataprocManagerImpl& m = *pImpl_;

    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QObjectList list = dockWidget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycle * pLifeCycle = dynamic_cast<adplugin::LifeCycle *>( obj );
            if ( pLifeCycle ) {
                pLifeCycle->OnInitialUpdate();
            }
        }
    }
}

void
DataprocManager::OnFinalClose()
{
    DataprocManagerImpl& m = *pImpl_;

    QList< QDockWidget *> dockWidgets = m.mainWindow_->dockWidgets();
  
    foreach ( QDockWidget * dockWidget, dockWidgets ) {
        QObjectList list = dockWidget->children();
        foreach ( QObject * obj, list ) {
            adplugin::LifeCycle * pLifeCycle = dynamic_cast<adplugin::LifeCycle *>( obj );
            if ( pLifeCycle ) {
                pLifeCycle->OnFinalClose();
            }
        }
    }
}

///////////////////////
DataprocManagerImpl::~DataprocManagerImpl()
{
}

DataprocManagerImpl::DataprocManagerImpl() : mainWindow_(0)
{
}

void
DataprocManagerImpl::init()
{
  mainWindow_ = new Utils::FancyMainWindow;
  if ( mainWindow_ ) {
    mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    mainWindow_->setDocumentMode( true );
    
  }
}

void
DataprocManager::setSimpleDockWidgetArrangement()
{
  class setTrackingEnabled {
    Utils::FancyMainWindow& w_;
  public:
    setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
    ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
  };

  DataprocManagerImpl& m = *pImpl_;
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

  for ( unsigned int i = 1; i < m.dockWidgetVec_.size(); ++i )
    m.mainWindow_->tabifyDockWidget( m.dockWidgetVec_[0], m.dockWidgetVec_[i] );
}
