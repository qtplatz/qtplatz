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
#include <adplugin/plugin_ptr.hpp>
#include <vector>
#include <string>
#include <filesystem>

// class QLibrary;
// class QString;
// class QStringList;

namespace boost {
    namespace dll { class shared_library; }
}

namespace adplugin {

    class plugin;
    class ADPLUGINSHARED_EXPORT loader;

    class loader {

    public:
        static void populate( const std::filesystem::path& );

        static std::string library_filename( const char * library );

        static bool load_config( const wchar_t * directory, const wchar_t * config_filename );

        static std::wstring config_fullpath( const std::wstring& apppath, const std::wstring& config_filename );

        static std::string debug_suffix();
        static std::filesystem::path shared_directory();
        static std::filesystem::path plugin_directory();

        //
        // static adplugin::plugin * loadLibrary( const QString& libname, const QStringList& paths );
        static adplugin::plugin * loadLibrary( const std::string& stem );
        static boost::dll::shared_library loadLibrary( const std::string& stem, std::error_code& ec );
    };

}
