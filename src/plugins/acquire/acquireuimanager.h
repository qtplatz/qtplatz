// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ACQUIREUIMANAGER_H
#define ACQUIREUIMANAGER_H

#include <QObject>
#include <QWidget>

namespace Utils { class FancyMainWindow; }
class QDockWidget;
class QAction;
class QMainWindow;

namespace Acquire {

  namespace internal {

    struct AcquireManagerActions;
    struct AcquireUIManagerData;

    //------------
    //------------
    class AcquireUIManager : public QObject {
      Q_OBJECT
    public:
      ~AcquireUIManager();
      explicit AcquireUIManager(QObject *parent = 0);
      
      QMainWindow * mainWindow() const;
      void init();
      void setSimpleDockWidgetArrangement();

      //
    signals:

    public slots:

    private:
      AcquireUIManagerData * d_;

    public:
      const AcquireManagerActions& acquireManagerActions() const;

    };

  }
}

#endif // ACQUIREUIMANAGER_H
