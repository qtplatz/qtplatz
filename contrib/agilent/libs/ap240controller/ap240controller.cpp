/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "session.hpp"
#include <adacquire/manager.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/visitor.hpp>
#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>


extern "C" {
    DECL_EXPORT adplugin::plugin * adplugin_plugin_instance();
}

namespace ap240controller {

    class factory : public adplugin::plugin
                  , public adacquire::manager {

        static std::once_flag flag;
        static std::shared_ptr<factory> instance_;

    public:

        static adplugin::plugin * instance() {
            std::call_once( flag, [&] () { instance_ = std::make_shared< factory >(); } );
            return instance_.get();
        }

        // adacquire::manager impl
        adacquire::Instrument::Session * session( const char * token ) override {
            return ap240controller::Instrument::Session::instance();
        }

        // adplugin impl

        const char * iid() const { return "com.ms-cheminfo.qtplatz.adplugins.ap240controller"; }

        // Linux may fail with dynamic_cast<> when shared library was dlopen'nd
        void * query_interface_workaround( const char * typenam ) override {
            if ( std::strcmp( typenam, typeid( adacquire::manager ).name() ) == 0 )
                return static_cast< adacquire::manager *>(this);
            return 0;
        }
        
        void accept( adplugin::visitor& v, const char * adplugin ) {
            v.visit( this, adplugin );
        }
    };

    std::shared_ptr<factory> factory::instance_( 0 );
    std::once_flag factory::flag;
};

adplugin::plugin *
adplugin_plugin_instance()
{
    return ap240controller::factory::instance();
}

