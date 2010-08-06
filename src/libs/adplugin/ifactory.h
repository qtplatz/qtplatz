// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adplugin_global.h"
#include <QObject>

namespace adplugin {

	class ADPLUGINSHARED_EXPORT IFactory : public QObject {
		Q_OBJECT
	public:
		explicit IFactory(QObject *parent = 0) : QObject( parent ) {}

		virtual QWidget * create_widget( const char * iid, QWidget * parent = 0 ) = 0;
		virtual QObject * create_object( const char * iid, QObject * parent = 0 ) = 0;

    signals:

	public slots:

	};
}

Q_DECLARE_INTERFACE( adplugin::IFactory, "org.adplugin.IFactory/1.0" );

