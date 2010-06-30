//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "sequencemanager.h"
#include <utils/fancymainwindow.h>
#include <utils/styledbar.h>
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
SequenceManager::init()
{
  mainWindow_ = new Utils::FancyMainWindow;
  if ( mainWindow_ ) {
    mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
    mainWindow_->setDocumentMode( true );
    
  }

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
  edit7->setWindowTitle( tr("Chromatogram") );
  
  QWidget * edit8 = new QTextEdit( "Edit 6" );
  edit8->setWindowTitle( tr("Report") );

    QDockWidget * dock1 = mainWindow_->addDockForWidget( edit1 );
    QDockWidget * dock2 = mainWindow_->addDockForWidget( edit2 );
    QDockWidget * dock3 = mainWindow_->addDockForWidget( edit3 );
    QDockWidget * dock4 = mainWindow_->addDockForWidget( edit4 );
    QDockWidget * dock5 = mainWindow_->addDockForWidget( edit5 );
    QDockWidget * dock6 = mainWindow_->addDockForWidget( edit6 );
    QDockWidget * dock7 = mainWindow_->addDockForWidget( edit7 );
    QDockWidget * dock8 = mainWindow_->addDockForWidget( edit8 );
    
    dockWidgetVec_.push_back( dock1 );
    dockWidgetVec_.push_back( dock2 );
    dockWidgetVec_.push_back( dock3 );
    dockWidgetVec_.push_back( dock4 );
    dockWidgetVec_.push_back( dock5 );
    dockWidgetVec_.push_back( dock6 );
    dockWidgetVec_.push_back( dock7 );
    dockWidgetVec_.push_back( dock8 );

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
