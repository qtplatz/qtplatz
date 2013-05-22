// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC Project
//////////////////////////////////////////////

#ifndef BATCHPROCPLUGIN_H
#define BATCHPROCPLUGIN_H

#include <extensionsystem/iplugin.h>

namespace Batchproc {
    namespace internal {

      class BatchprocPlugin : public ExtensionSystem::IPlugin {
	Q_OBJECT
	  ;
      public:
	~BatchprocPlugin();
	explicit BatchprocPlugin();
        bool initialize(const QStringList &arguments, QString *error_message);
	
      signals:

      public slots:

      };
      //-------------------
    }
}

#endif // BATCHPROCPLUGIN_H
