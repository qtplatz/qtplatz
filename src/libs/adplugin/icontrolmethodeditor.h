// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ICONTROLMETHODEDITOR_H
#define ICONTROLMETHODEDITOR_H

#include "adplugin_global.h"
#include <QWidget>

namespace adplugin {
    namespace ui {

		class ADPLUGINSHARED_EXPORT IControlMethodEditor : public QWidget {
			Q_OBJECT
		public:
			explicit IControlMethodEditor(QWidget *parent = 0) : QWidget( parent ) {}

        signals:

			public slots:

		};

    }
}

#endif // ICONTROLMETHODEDITOR_H
