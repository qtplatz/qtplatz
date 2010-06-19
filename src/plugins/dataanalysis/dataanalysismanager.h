#ifndef DATAANALYSISMANAGER_H
#define DATAANALYSISMANAGER_H

#include <boost/smart_ptr.hpp>
#include <vector>
#include <QtCore/QObject>

namespace Utils {
    class FancyMainWindow;
}

class QDockWidget;
class QWidget;
class QAction;

namespace DataAnalysis {
  namespace Internal {
    
    class DataAnalysisManager : public QObject {
      Q_OBJECT
    public:
      ~DataAnalysisManager();
      DataAnalysisManager();
      Utils::FancyMainWindow * mainWindow() const;
      void init();
      void setSimpleDockWidgetArrangement();
    private:
      Utils::FancyMainWindow * mainWindow_;
      boost::shared_ptr< QWidget > breakWindow_;
      boost::shared_ptr< QWidget > outputWindow_;

      QDockWidget * breakDock_;
      QDockWidget * outputDock_;
      std::vector< QAction * > actions_;
    };
    
  }
}

#endif // DATAANALYSISMANAGER_H
