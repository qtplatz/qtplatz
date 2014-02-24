// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#pragma once

#include "adplugin_global.h"
#include <QObject>

namespace adplugin {

    class ADPLUGINSHARED_EXPORT widget_factory {
	public:
        virtual ~widget_factory();
        widget_factory();

        virtual QWidget * create_widget( const wchar_t * iid, QWidget * parent = 0 ) = 0;
        virtual QObject * create_object( const wchar_t * iid, QObject * parent = 0 ) = 0;

        virtual void release() = 0;

        // static widget_factory * find_factory( const char * iid, const char * clsid = 0 );
        static QWidget * create( const char * wiid, const char * clsid = 0, QWidget * parent = 0 );
        static QWidget * create( const wchar_t * wiid, const char * clsid = 0, QWidget * parent = 0 );
    };

}
