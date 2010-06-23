//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "acquireuimanager.h"
#include "acquireactions.h"
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

//const AcquireManagerActions&
//AcquireUIManager::acquireManagerActions() const
//{
//  return d_->actions_;
//}


void
AcquireUIManager::init()
{
  if ( ! d_ )
    return;
  
  Acquire::internal::AcquireUIManagerData& m = *d_;
  
  m.mainWindow_ = new Utils::FancyMainWindow;
  if ( d_ && m.mainWindow_ ) {
    m.mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    m.mainWindow_->setDocumentMode( true );
    
    QWidget * edit1 = new QTextEdit( "Sequence" );
    edit1->setWindowTitle( tr("Sequence") );

    QWidget * edit2 = new QTextEdit( "Log" );
    edit2->setWindowTitle( tr("Log Book") );

    QWidget * edit3 = new QTextEdit( "MS" );
    edit3->setWindowTitle( tr("Autosampler") );

    QWidget * edit4 = new QTextEdit( "Edit4" );
    edit4->setWindowTitle( tr("Mass Spectrometer") );

    QWidget * edit5 = new QTextEdit( "Edit 5" );
    edit5->setWindowTitle( tr("Agilnet 1290") );

    QWidget * edit6 = new QTextEdit( "Edit 6" );
    edit6->setWindowTitle( tr("All") );

    QDockWidget * dock1 = m.mainWindow_->addDockForWidget( edit1 );
    QDockWidget * dock2 = m.mainWindow_->addDockForWidget( edit2 );
    QDockWidget * dock3 = m.mainWindow_->addDockForWidget( edit3 );
    QDockWidget * dock4 = m.mainWindow_->addDockForWidget( edit4 );
    QDockWidget * dock5 = m.mainWindow_->addDockForWidget( edit5 );
    QDockWidget * dock6 = m.mainWindow_->addDockForWidget( edit6 );
    
    m.dockWidgetVec_.push_back( dock1 );
    m.dockWidgetVec_.push_back( dock2 );
    m.dockWidgetVec_.push_back( dock3 );
    m.dockWidgetVec_.push_back( dock4 );
    m.dockWidgetVec_.push_back( dock5 );
    m.dockWidgetVec_.push_back( dock6 );
    
    // todo
    // set actions
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
