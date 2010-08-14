// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ACQUIREPLUGIN_H
#define ACQUIREPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <adinterface/controlserverC.h>

class QToolButton;
class QAction;

namespace Acquire {
  namespace internal {

    class AcquireUIManager;
	class AcquireImpl;

    //------------
    class AcquirePlugin : public ExtensionSystem::IPlugin {
      Q_OBJECT
    public:
      ~AcquirePlugin();
      AcquirePlugin();

      // implement IPlugin
      virtual bool initialize(const QStringList &arguments, QString *error_message);
      virtual void extensionsInitialized();
      virtual void shutdown();

    private slots:
		void actionConnect();
		void actionRunStop();

    private:
      AcquireUIManager * manager_;
	  AcquireImpl * pImpl_;

      QAction * actionConnect_;
      QAction * actionRunStop_;
      QAction * action3_;
      QAction * action4_;
      QAction * action5_;

      void action1();
      void action2();
      void action3();
      void action4();
      void action5();

      void initialize_actions();

      ControlServer::Session_var session_;

    public:
      static QToolButton * toolButton( QAction * action );
    };
  }
}

#endif // ACQUIREPLUGIN_H
