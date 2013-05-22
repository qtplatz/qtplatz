// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef IMONITOR_H
#define IMONITOR_H

#include "adplugin_global.h"
#include "lifecycle.hpp"
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
