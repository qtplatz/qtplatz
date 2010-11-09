//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "sequencemanager.h"
#include <adportable/configuration.h>
#include <adplugin/adplugin.h>
#include <adplugin/lifecycle.h>
#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
#include <qtwrapper/qstring.h>
#include <QtCore/QHash>
#include <QString>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolBar>
#include <QTextEdit>

using namespace sequence::internal;

SequenceManager::~SequenceManager()
{
}

SequenceManager::SequenceManager(QObject *parent) :
    QObject(parent)
    , mainWindow_(0)
{
}

QMainWindow *
SequenceManager::mainWindow() const
{
    return mainWindow_;
}

void
SequenceManager::init( const std::wstring& apppath
                      , adportable::Configuration& acquire_config
                      , adportable::Configuration& dataproc_config )
{
    mainWindow_ = new Utils::FancyMainWindow;
    if ( mainWindow_ ) {
        mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
        mainWindow_->setDocumentMode( true );
    }

    Q_UNUSED( acquire_config );

    const adportable::Configuration * pTab
        = adportable::Configuration::find( dataproc_config, L"ProcessMethodEditors" );
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
                    QDockWidget * dock = mainWindow_->addDockForWidget( pWidget );
                    dockWidgetVec_.push_back( dock );

                } else {
                    QWidget * edit = new QTextEdit( "Edit" );
                    edit->setWindowTitle( qtwrapper::qstring( it->title() ) );
                    QDockWidget * dock = mainWindow_->addDockForWidget( edit );
                    dockWidgetVec_.push_back( dock );
                }
            }
        }
    }            
}

void
SequenceManager::setSimpleDockWidgetArrangement()
{
  class setTrackingEnabled {
    Utils::FancyMainWindow& w_;
  public:
    setTrackingEnabled( Utils::FancyMainWindow& w ) : w_(w) { w_.setTrackingEnabled( false ); }
    ~setTrackingEnabled() {  w_.setTrackingEnabled( true ); }
  };

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

  for ( unsigned int i = 1; i < dockWidgetVec_.size(); ++i )
    mainWindow_->tabifyDockWidget( dockWidgetVec_[0], dockWidgetVec_[i] );
}

void
SequenceManager::OnInitialUpdate()
{
    QList< QDockWidget *> dockWidgets = mainWindow_->dockWidgets();
  
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
SequenceManager::OnFinalClose()
{
    QList< QDockWidget *> dockWidgets = mainWindow_->dockWidgets();
  
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
