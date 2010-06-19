#ifndef DATAANALYSISMANAGER_H
#define DATAANALYSISMANAGER_H

#include <boost/smart_ptr.hpp>

namespace Utils {
    class FancyMainWindow;
}

class QDocWidget;
class QWidget;

namespace DataAnalysis {
  namespace Internal {
    
    class DataAnalysisManager {
    public:
      ~DataAnalysisManager();
      DataAnalysisManager();
      Utils::FancyMainWindow * mainWindow() const;
      void init();
    private:
      Utils::FancyMainWindow * mainWindow_;
      boost::shared_ptr< QWidget > breakWindow_;
      boost::shared_ptr< QWidget > outputWindow_;
      boost::shared_ptr< QWidget > registerWindow_;
      boost::shared_ptr< QWidget > watchWindow_;
    };
    
  }
}

#endif // DATAANALYSISMANAGER_H
