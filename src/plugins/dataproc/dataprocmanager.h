// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <QObject>
#include <boost/smart_ptr.hpp>

class QMainWindow;

namespace dataproc {
  namespace internal {

    class DataprocManagerImpl;

    class DataprocManager : public QObject {
      Q_OBJECT
    public:
      explicit DataprocManager(QObject *parent = 0);

      QMainWindow * mainWindow() const;
      void init();
      void setSimpleDockWidgetArrangement();
      
    signals:
      
    public slots:

    private:
      boost::shared_ptr<DataprocManagerImpl> pImpl_;
    };

  }
}

