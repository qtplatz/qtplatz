// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef SEQUENCEPLUGIN_H
#define SEQUENCEPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>

namespace adportable {
    class Configuration;
}

namespace sequence {
  namespace internal {

    class SequenceManager;

    class SequencePlugin : public ExtensionSystem::IPlugin {
      Q_OBJECT;
    public:
      ~SequencePlugin();
      explicit SequencePlugin();

      bool initialize(const QStringList& arguments, QString* error_message);
      void extensionsInitialized();
      void shutdown();

    signals:

    public slots:

    private:
      boost::shared_ptr<SequenceManager> manager_;
      QWidget * CreateSequenceWidget( const std::wstring&, const adportable::Configuration& );
    };
    //------
  }
}

#endif // SEQUENCEPLUGIN_H
