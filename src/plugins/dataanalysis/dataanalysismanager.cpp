#include "dataanalysismanager.h"
#include <utils/fancymainwindow.h>
#include <QTabWidget>

using namespace DataAnalysis;
using namespace DataAnalysis::Internal;

DataAnalysisManager::~DataAnalysisManager()
{
}

DataAnalysisManager::DataAnalysisManager() : mainWindow_(0)
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
}



