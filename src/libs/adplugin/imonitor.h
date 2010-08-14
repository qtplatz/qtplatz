// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef IMONITOR_H
#define IMONITOR_H

#include "adplugin_global.h"
#include "lifecycle.h"
#include <QWidget>

namespace adplugin {

    namespace ui {
        class ADPLUGINSHARED_EXPORT IMonitor : public QWidget // this must be on top of inherit list
                                             , public LifeCycle {
            Q_OBJECT
        public:
            explicit IMonitor(QWidget *parent = 0) : QWidget( parent ) {}
            
        signals:
            
	    public slots:
                
        };
    }
}

Q_DECLARE_INTERFACE( adplugin::ui::IMonitor, "org.adplugin.ui.imonitor/1.0" );

#endif // IMONITOR_H
