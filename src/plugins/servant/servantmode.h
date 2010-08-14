// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef SERVANTMODE_H
#define SERVANTMODE_H

#include <coreplugin/basemode.h>

namespace servant {
  namespace internal {

	class ServantMode : public Core::BaseMode {
	  Q_OBJECT
	public:
      ~ServantMode();
	  explicit ServantMode(QObject *parent = 0);
	  
	signals:
	  
	public slots:
	  
	};
	
  }
}

#endif // SERVANTMODE_H
