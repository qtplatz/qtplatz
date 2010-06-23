// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef ANALYSISMANAGER_H
#define ANALYSISMANAGER_H

#include <QObject>
#include <boost/smart_ptr.hpp>

class QMainWindow;

namespace Analysis {
  namespace internal {

    class AnalysisManagerImpl;

    class AnalysisManager : public QObject {
      Q_OBJECT
    public:
      explicit AnalysisManager(QObject *parent = 0);

      QMainWindow * mainWindow() const;
      void init();
      void setSimpleDockWidgetArrangement();
      
    signals:
      
    public slots:

    private:
      boost::shared_ptr<AnalysisManagerImpl> pImpl_;
    };

  }
}

#endif // ANALYSISMANAGER_H
