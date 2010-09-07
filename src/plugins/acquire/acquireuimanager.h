// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef ACQUIREUIMANAGER_H
#define ACQUIREUIMANAGER_H

#include <QObject>
#include <QWidget>
#pragma warning(disable:4996)
#pragma warning(disable:4805)
#include <adinterface/receiverC.h>
#pragma warning(default:4805)
#pragma warning(default:4996)

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

      void OnInitialUpdate();
      void OnFinalClose();

      //
    signals:
        void signal_eventLog( QString );
        void signal_message( unsigned long msg, unsigned long value );
        void signal_debug_print( unsigned long priority, unsigned long category, QString text );

    public slots:
        void handle_message( unsigned long msg, unsigned long value );
        void handle_eventLog( const ::EventLog::LogMessage& );
        void handle_shutdown();
        void handle_debug_print( unsigned long priority, unsigned long category, QString text );

    private:
      AcquireUIManagerData * d_;

    public:
      //const AcquireManagerActions& acquireManagerActions() const;

    };

  }
}

#endif // ACQUIREUIMANAGER_H
