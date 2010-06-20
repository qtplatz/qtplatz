//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////

#include "acquireplugin.h"
#include "acquiremode.h"
#include "acquireuimanager.h"
#include <utils/fancymainwindow.h>

#include <libadwidgets/tracewidget.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <QtCore/qplugin.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/rightpane.h>
#include <utils/styledbar.h>
#include <QtGui/QHBoxLayout>
#include <QtGui/QBoxLayout>
#include <QtGui/QToolButton>
#include <QtGui/QLabel>

using namespace Acquire;
using namespace Acquire::internal;

AcquirePlugin::~AcquirePlugin()
{
  delete manager_;
}

AcquirePlugin::AcquirePlugin() : manager_(0)
{
}

bool
AcquirePlugin::initialize(const QStringList &arguments, QString *error_message)
{
  Q_UNUSED(arguments);
  Q_UNUSED(error_message);
  Core::ICore * core = Core::ICore::instance();
  if ( ! core )
    return false;

  QList<int> context;
  context.append( core->uniqueIDManager()->uniqueIdentifier( QLatin1String("Acquire.MainView") ) );

  AcquireMode * mode = new AcquireMode(this);
  if ( ! mode )
    return false;
  mode->setContext( context );

  manager_ = new AcquireUIManager(0);
  if ( manager_ )
    manager_->init();

  do {
    /*
    Core::MiniSplitter * splitter = new Core::MiniSplitter;
    if ( splitter ) {
      splitter->addWidget( manager_->mainWindow() );
      //splitter->addWidget( new Core::OutputPanePlaceHolder( mode ) );
      //splitter->setStretchFactor( 0, 10 );
      //splitter->setStretchFactor( 1, 0 );
      splitter->setOrientation( Qt::Vertical );
    }
    */

    Core::MiniSplitter * splitter2 = new Core::MiniSplitter;
    if ( splitter2 ) {
      splitter2->addWidget( new Core::NavigationWidgetPlaceHolder( mode ) );
      splitter2->addWidget( manager_->mainWindow() ); // splitter );
      splitter2->setStretchFactor( 0, 0 );
      splitter2->setStretchFactor( 1, 1 );
    }
      
    Utils::StyledBar * toolBar = new Utils::StyledBar;
    if ( toolBar ) {
      toolBar->setProperty( "topBorder", true );
      QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
      toolBarLayout->setMargin(0);
      toolBarLayout->setSpacing(0);
      toolBarLayout->addWidget( new QToolButton );
      toolBarLayout->addWidget( new QLabel( tr("Threads:") ) );
      toolBarLayout->addWidget( new QToolButton );
      toolBarLayout->addWidget( new QLabel( tr("ABC") ) );
      toolBarLayout->addWidget( new QToolButton );
      toolBarLayout->addWidget( new QLabel( tr("DEF") ) );
    }

    Core::MiniSplitter * rightPaneHSplitter = new Core::MiniSplitter;
    if ( rightPaneHSplitter ) {
      rightPaneHSplitter->addWidget( new adil::TraceWidget );
      rightPaneHSplitter->addWidget( new Core::RightPanePlaceHolder( mode ) );
      //rightPaneSplitter->setStretchFactor( 0, 1 );
      //rightPaneSplitter->setStretchFactor( 1, 0 );
    }

    QWidget* centralWidget = new QWidget;
    manager_->mainWindow()->setCentralWidget( centralWidget );
      
    QBoxLayout * toolBarAddingLayout = new QVBoxLayout( centralWidget );
    toolBarAddingLayout->setMargin(0);
    toolBarAddingLayout->setSpacing(0);
    toolBarAddingLayout->addWidget( rightPaneHSplitter );
    toolBarAddingLayout->addWidget( toolBar );
    
    /////////////////////////////////
      mode->setWidget( splitter2 );
      //////////////////////////////
  } while(0);
  
  manager_->setSimpleDockWidgetArrangement();
  addAutoReleasedObject(mode);

  return true;
}

void
AcquirePlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN( AcquirePlugin )
