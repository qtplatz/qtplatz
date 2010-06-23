//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "analysismanager.h"
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

using namespace Analysis;
using namespace Analysis::internal;

namespace Analysis {
  namespace internal {
    class AnalysisManagerImpl : boost::noncopyable {
    public:
      ~AnalysisManagerImpl();
      AnalysisManagerImpl();

      Utils::FancyMainWindow * mainWindow_;
      void init();

    public:
      std::vector< QDockWidget * > dockWidgetVec_;

    };
  }
}

AnalysisManager::AnalysisManager(QObject *parent) :
    QObject(parent)
    , pImpl_( new AnalysisManagerImpl() )
{
}

QMainWindow *
AnalysisManager::mainWindow() const
{
    return pImpl_->mainWindow_;
}

void
AnalysisManager::init()
{
  pImpl_->init();

  QWidget * edit1 = new QTextEdit( "Sequence" );
  edit1->setWindowTitle( tr("Centroid") );
  
  QWidget * edit2 = new QTextEdit( "Log" );
  edit2->setWindowTitle( tr("Elemental Composition") );
  
  QWidget * edit3 = new QTextEdit( "MS" );
  edit3->setWindowTitle( tr("Lockmass") );
  
  QWidget * edit4 = new QTextEdit( "Edit4" );
  edit4->setWindowTitle( tr("Isotope") );
  
  QWidget * edit5 = new QTextEdit( "Edit 5" );
  edit5->setWindowTitle( tr("MS Calibration") );
  
  QWidget * edit6 = new QTextEdit( "Edit 6" );
  edit6->setWindowTitle( tr("Targeting") );
  
  QWidget * edit7 = new QTextEdit( "Edit 6" );
  edit6->setWindowTitle( tr("Chromatogram") );
  
  QWidget * edit8 = new QTextEdit( "Edit 6" );
  edit6->setWindowTitle( tr("Report") );

  AnalysisManagerImpl& m = *pImpl_;

    QDockWidget * dock1 = m.mainWindow_->addDockForWidget( edit1 );
    QDockWidget * dock2 = m.mainWindow_->addDockForWidget( edit2 );
    QDockWidget * dock3 = m.mainWindow_->addDockForWidget( edit3 );
    QDockWidget * dock4 = m.mainWindow_->addDockForWidget( edit4 );
    QDockWidget * dock5 = m.mainWindow_->addDockForWidget( edit5 );
    QDockWidget * dock6 = m.mainWindow_->addDockForWidget( edit6 );
    QDockWidget * dock7 = m.mainWindow_->addDockForWidget( edit7 );
    QDockWidget * dock8 = m.mainWindow_->addDockForWidget( edit8 );
    
    m.dockWidgetVec_.push_back( dock1 );
    m.dockWidgetVec_.push_back( dock2 );
    m.dockWidgetVec_.push_back( dock3 );
    m.dockWidgetVec_.push_back( dock4 );
    m.dockWidgetVec_.push_back( dock5 );
    m.dockWidgetVec_.push_back( dock6 );
    m.dockWidgetVec_.push_back( dock7 );
    m.dockWidgetVec_.push_back( dock8 );

}

///////////////////////
AnalysisManagerImpl::~AnalysisManagerImpl()
{
}

AnalysisManagerImpl::AnalysisManagerImpl() : mainWindow_(0)
{
}

void
AnalysisManagerImpl::init()
{
  mainWindow_ = new Utils::FancyMainWindow;
  if ( mainWindow_ ) {
    mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    mainWindow_->setDocumentMode( true );
    
  }
}

