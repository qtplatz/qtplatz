// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adplugin_global.h"
#include <QObject>

namespace adplugin {

    class ADPLUGINSHARED_EXPORT ifactory {
	public:
        ifactory() {}
        virtual ~ifactory() {}

		virtual QWidget * create_widget( const wchar_t * iid, QWidget * parent = 0 ) = 0;
		virtual QObject * create_object( const wchar_t * iid, QObject * parent = 0 ) = 0;
        virtual void release() = 0;
	};

}
