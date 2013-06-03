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
#include "plugin_ptr.hpp"
#include <vector>

namespace adplugin {

    class plugin;

    class ADPLUGINSHARED_EXPORT loader {
    public:
		static void populate( const wchar_t * directory );
        static void load( const wchar_t * library_filename );
        static void unload( const wchar_t * library_filename );
        static plugin_ptr select_iid( const char * iid );
        static plugin_ptr select_clsid( const char * clsid ); // return first match only
        static size_t select_iids( const char * regex, std::vector< plugin_ptr >& );
        static size_t select_clsids( const char * clsid, std::vector< plugin_ptr >& );
        // 
        static bool load_config( const wchar_t * directory, const wchar_t * config_filename );
    };

}


