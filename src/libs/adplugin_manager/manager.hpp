// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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
#include <adplugin/plugin_ptr.hpp>
#include <vector>
#include <memory>
#include <compiler/pragma_warning.hpp>

class QString;
class QObject;
class QWidget;
class QLibrary;

namespace adportable {
    class Configuration;
    class Component;
}

namespace adplugin {

    namespace internal { class manager_data; }

    class ADPLUGINSHARED_EXPORT manager {

        class data;
        data * d_;
        
        pragma_msvc_warning_push_disable_4251
        static std::unique_ptr< manager > instance_;
        pragma_msvc_warning_pop


    protected:
        manager();
        ~manager();

    public:

        static manager * instance();
        static void dispose();

        bool install( QLibrary&, const std::string& adpluginspec );

		void populated();

        plugin_ptr select_iid( const char * regex );

        plugin_ptr select_clsid( const char * regex );

        size_t select_iids( const char * regex, std::vector< plugin_ptr >& vec );

        size_t select_clsids( const char * regex, std::vector< plugin_ptr >& vec );

    private:
        friend std::unique_ptr< manager >::deleter_type;
    };
}
