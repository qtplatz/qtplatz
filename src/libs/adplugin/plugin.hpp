// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013 MS-Cheminformatics LLC
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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
#include <string>

namespace adplugin {

    class visitor;
    namespace internal { class manager_data; }

    class ADPLUGINSHARED_EXPORT plugin {
        std::string clsid_; // unique id for dll as full path to "*.adplugin"
        friend class internal::manager_data;
	protected:
		virtual ~plugin() {}
    public:
        plugin();
		virtual void dispose() { delete this; }
        virtual void accept( visitor&, const char * adplugin ) = 0;
        virtual const char * iid() const = 0;
        virtual const char * clsid() const { return clsid_.c_str(); } // adplugin name
        template<typename T> T* query_interface() { return dynamic_cast<T*>(this); }
    };

}


