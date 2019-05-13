/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "processreactor.hpp"
#include <adplugin/plugin.hpp>
#include <mutex>

namespace infitofprocessor {

    class infitofprocessor_plugin : public adplugin::plugin {

        infitofprocessor_plugin( const infitofprocessor_plugin& ) = delete;
#if _MSC_VER >= 1900
    public:
#endif
        infitofprocessor_plugin() {}
    public:
        ~infitofprocessor_plugin() {
        }
        
        static std::shared_ptr< infitofprocessor_plugin > instance_;
        
        static infitofprocessor_plugin * instance() {
            static std::once_flag flag;
            std::call_once( flag, [] () {
                    struct make_shared_enabler : public infitofprocessor_plugin {};
                    instance_ = std::make_shared< make_shared_enabler >();
                } );
            
            return instance_.get();
        }
        
        // plugin
        void * query_interface_workaround( const char * ) override { return 0; }
        
        void accept( adplugin::visitor&, const char * adplugin ) override;
        
        const char * iid() const override { return "com.ms-cheminfo.qtplatz.adprocessor.infitofprocessor"; }
    };
    
    std::shared_ptr< infitofprocessor_plugin > infitofprocessor_plugin::instance_;
}

extern "C" {
    DECL_EXPORT adplugin::plugin * adplugin_plugin_instance();
}

adplugin::plugin *
adplugin_plugin_instance()
{
    return infitofprocessor::infitofprocessor_plugin::instance();
}

using namespace infitofprocessor;

void
infitofprocessor_plugin::accept( adplugin::visitor&, const char * adplugin )
{
    ProcessReactor::instance()->initialSetup();
}

