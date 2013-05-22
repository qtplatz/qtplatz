// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef SERVANTUIMANAGER_H
#define SERVANTUIMANAGER_H

#include <QObject>
#include <QWidget>

namespace Utils {
  class FancyMainWindow; 
}

class QDockWidget;
class QAction;
class QMainWindow;

namespace servant {
  namespace internal {

	class ServantUIManager : public QObject {
	  Q_OBJECT
	public:
      ~ServantUIManager();
	  explicit ServantUIManager(QObject *parent = 0);
  
      QMainWindow * mainWindow() const;
      void init();
	  void setSimpleDockWidgetArrangement();

	private:
       QMainWindow * mainWindow_;
	  
	signals:
	  
    public slots:

	};

  }
}

#endif // SERVANTUIMANAGER_H
