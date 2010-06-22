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

    namespace ui {

      struct TimeTrace {
      };
      
      struct Spectrum {
      };

      struct TabbedPane {
      };
    }

    template<class T, class Q = QWidget> struct QWidget_t : public Q {
      T t_;
    };

	typedef boost::variant< 
                QWidget_t<ui::TimeTrace, adil::ui::Dataplot>
                , QWidget_t<ui::Spectrum, adil::ui::Dataplot>
		, QWidget_t<ui::TabbedPane, QTabWidget> > widget_type;
  
    struct AcquireUIManagerData : boost::noncopyable {
      AcquireUIManagerData() : mainWindow_(0) {}
      
      Utils::FancyMainWindow* mainWindow_;

	  //QWidget_t<ui::TimeTrace, adil::TraceWidget>  timeTraceWidget_;
	  //QWidget_t<ui::Spectrum,  adil::TraceWidget>  spectrumWidget_;
	  //QWidget_t<ui::TabbedPane, QWidget> tabbedWidget_;

      std::vector< QDockWidget * > dockWidgetVec_;
      AcquireManagerActions actions_;
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

const AcquireManagerActions&
AcquireUIManager::acquireManagerActions() const
{
  return d_->actions_;
}


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
    
    QWidget * edit1 = new QTextEdit( "Edit 1" );
    QWidget * edit2 = new QTextEdit( "Edit 2" );
    QWidget * edit3 = new QTextEdit( "Edit 3" );
    QWidget * edit4 = new QTextEdit( "Edit 4" );
    QWidget * edit5 = new QTextEdit( "Edit 5" );
    QWidget * edit6 = new QTextEdit( "Edit 6" );
    
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
    
    //m.dockWidgetVec_.push_back( m.mainWindow_->addDockForWidget( &m.spectrumWidget_ ) );
    //m.dockWidgetVec_.push_back( m.mainWindow_->addDockForWidget( &m.tabbedWidget_ ) );
    //m.dockWidgetVec_.push_back( m.mainWindow_->addDockForWidget( &m.tabbedWidget_ ) );
    
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
    //if ( dockWidget == m.dockWidgetVec_.front() )
    //  m.mainWindow_->addDockWidget( Qt::TopDockWidgetArea, dockWidget );
    //else
    m.mainWindow_->addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
    dockWidget->show();
  }

  for ( unsigned int i = 2; i < m.dockWidgetVec_.size(); ++i )
    m.mainWindow_->tabifyDockWidget( m.dockWidgetVec_[1], m.dockWidgetVec_[i] );

}





