// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ACQUIREPLUGIN_H
#define ACQUIREPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <adinterface/controlserverC.h>
#include <boost/smart_ptr.hpp>
#include <adplugin/qreceiver_i.h>

class QToolButton;
class QAction;

namespace adplugin {
   class QReceiver_i;
}

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

        void handle_message( Receiver::eINSTEVENT msg, unsigned long value );
        void handle_log( QByteArray );
        void handle_shutdown();
        void handle_debug_print( unsigned long priority, unsigned long category, QString text );
    signals:

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
      boost::scoped_ptr< adplugin::QReceiver_i > receiver_i_;

    public:
      static QToolButton * toolButton( QAction * action );
    };
  }
}

#endif // ACQUIREPLUGIN_H
