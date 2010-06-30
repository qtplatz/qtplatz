// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef SEQUENCEMANAGER_H
#define SEQUENCEMANAGER_H

#include <QObject>
#include <boost/smart_ptr.hpp>
#include <vector>

class QDockWidget;
class QMainWindow;

namespace Utils {
  class FancyMainWindow;
}

namespace sequence {
  namespace internal {

    class SequenceManager : public QObject {
      Q_OBJECT
    public:
      ~SequenceManager();
      explicit SequenceManager(QObject *parent = 0);

      QMainWindow * mainWindow() const;
      void init();
      void setSimpleDockWidgetArrangement();

    signals:

    public slots:

    private:
      Utils::FancyMainWindow * mainWindow_;
      std::vector< QDockWidget * > dockWidgetVec_;
    };

  }
}

#endif // SEQUENCEMANAGER_H
