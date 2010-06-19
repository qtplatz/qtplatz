#include "dataanalysismanager.h"
#include <utils/fancymainwindow.h>
#include <QTabWidget>
#include <QTreeView>
#include "outputwindow.h"
#include <QDockWidget>
#include <QAction>

using namespace DataAnalysis;
using namespace DataAnalysis::Internal;

DataAnalysisManager::~DataAnalysisManager()
{
}

DataAnalysisManager::DataAnalysisManager() : mainWindow_(0)
        , outputDock_(0)
        , breakDock_(0)
{
}

Utils::FancyMainWindow *
DataAnalysisManager::mainWindow() const
{
  return mainWindow_;
}

void
DataAnalysisManager::init()
{
    mainWindow_ = new Utils::FancyMainWindow;
    if ( mainWindow_ ) {
        mainWindow_->setTabPosition( Qt::AllDockWidgetAreas, QTabWidget::North );
        mainWindow_->setDocumentMode( true );
    }
    outputWindow_.reset( new OutputWindow );
    breakWindow_.reset( new QTreeView );

    breakWindow_->setWindowTitle( tr("Title") );

    outputDock_ = mainWindow_->addDockForWidget( outputWindow_.get() );
    breakDock_ = mainWindow_->addDockForWidget( breakWindow_.get() );

    actions_.push_back( new QAction( tr("Continue"), this ) );
    actions_.back()->setIcon( QIcon(":/dataanalysis/images/debugger_continue_small.png"));
}

void
DataAnalysisManager::setSimpleDockWidgetArrangement()
{
    mainWindow_->setTrackingEnabled( false );
    QList<QDockWidget *> dockWidgets = mainWindow_->dockWidgets();

    foreach( QDockWidget * dockWidget, dockWidgets ) {
        dockWidget->setFloating(false);
        mainWindow_->removeDockWidget(dockWidget);
    }

    foreach( QDockWidget * dockWidget, dockWidgets ) {
        if ( dockWidget == outputDock_ )
            mainWindow_->addDockWidget( Qt::TopDockWidgetArea, dockWidget );
        else
            mainWindow_->addDockWidget( Qt::BottomDockWidgetArea, dockWidget );
        dockWidget->show();
    }
    mainWindow_->setTrackingEnabled( true );
}


