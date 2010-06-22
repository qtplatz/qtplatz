// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ACQUIREPLUGIN_H
#define ACQUIREPLUGIN_H

#include <extensionsystem/iplugin.h>

class QToolButton;
class QAction;

namespace Acquire {
  namespace internal {

    class AcquireUIManager;

    //------------
    class AcquirePlugin : public ExtensionSystem::IPlugin {
      Q_OBJECT
    public:
      ~AcquirePlugin();
      AcquirePlugin();
      bool initialize(const QStringList &arguments, QString *error_message);
      void extensionsInitialized();
    private slots:

    private:
      AcquireUIManager * manager_;

      QAction * action1_;
      QAction * action2_;
      QAction * action3_;
      QAction * action4_;
      QAction * action5_;

      void action1();
      void action2();
      void action3();
      void action4();
      void action5();

      void initialize_actions();

    public:
      static QToolButton * toolButton( QAction * action );
    };
  }
}

#endif // ACQUIREPLUGIN_H
