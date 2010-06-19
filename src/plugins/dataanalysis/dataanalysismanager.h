#ifndef DATAANALYSISMANAGER_H
#define DATAANALYSISMANAGER_H

namespace Utils {
    class FancyMainWindow;
}

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
    };
    
  }
}

#endif // DATAANALYSISMANAGER_H
