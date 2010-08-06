// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <adplugin/ifactory.h>

namespace adtofms {

	class factory_impl : public adplugin::IFactory {
		Q_OBJECT
		Q_INTERFACES( adplugin::IFactory )
	public:
		explicit factory_impl(QObject *parent = 0);

		virtual QWidget * create_widget( const char * iid, QWidget * parent );
		virtual QObject * create_object( const char * iid, QObject * parent );
    signals:

    public slots:

	};

}

