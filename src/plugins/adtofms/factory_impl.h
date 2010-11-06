// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <adplugin/ifactory.h>

namespace adtofms {

	class factory_impl : public adplugin::ifactory {
	public:
		explicit factory_impl();

		virtual QWidget * create_widget( const wchar_t * iid, QWidget * parent );
		virtual QObject * create_object( const wchar_t * iid, QObject * parent );
        virtual void release();
    signals:

    public slots:

	};

}

