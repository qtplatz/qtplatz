//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#include "acquireuimanager.h"
#include <boost/variant.hpp>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>

#include <libadwidgets/tracewidget.h>
#include <QDockWidget>
#include <utils/fancymainwindow.h>


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

    typedef boost::variant< QWidget_t<ui::TimeTrace, adil::TraceWidget>
			    , QWidget_t<ui::Spectrum, adil::TraceWidget>
			    , QWidget_t<ui::TabbedPane, QWidget> > widget_type;
  
    struct AcquireUIManagerData : boost::noncopyable {
      AcquireUIManagerData() : mainWindow_(0) {}
      
      Utils::FancyMainWindow* mainWindow_;

      QWidget_t<ui::TimeTrace, adil::TraceWidget>  timeTraceWidget_;
      QWidget_t<ui::Spectrum,  adil::TraceWidget>  spectrumWidget_;
      QWidget_t<ui::TabbedPane, QWidget> tabbedWidget_;

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
  
  m.mainWindow_ = new Utils::FancyMainWindow;
  if ( d_ && m.mainWindow_ ) {
    m.mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    m.mainWindow_->setDocumentMode( true );
  }

  m.timeTraceWidget_.setWindowTitle( tr("Time Trace") );
  m.spectrumWidget_.setWindowTitle( tr("Spectrum") );
  m.tabbedWidget_.setWindowTitle( tr("Tab") );

  m.dockWidgetVec_.push_back( m.mainWindow_->addDockForWidget( &m.timeTraceWidget_ ) );
  m.dockWidgetVec_.push_back( m.mainWindow_->addDockForWidget( &m.spectrumWidget_ ) );
  m.dockWidgetVec_.push_back( m.mainWindow_->addDockForWidget( &m.tabbedWidget_ ) );

  // todo
  // set actions
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
    if ( dockWidget == m.dockWidgetVec_.front() )
      m.mainWindow_->addDockWidget( Qt::TopDockWidgetArea, dockWidget );
    else
      m.mainWindow_->addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
    dockWidget->show();
  }
}
