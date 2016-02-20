/**************************************************************************
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "massspectrometer.hpp"
#include "constants.hpp"
#include <adcontrols/massspectrometer_factory.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adplugin/plugin.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <memory>
#include <mutex>

namespace adcontrols { class datafile; }

namespace adspectrometer {

    class adspectrometer_plugin : public adplugin::plugin {
        adspectrometer_plugin( const adspectrometer_plugin& ) = delete;
#if _MSC_VER >= 1900
    public:
#endif
        adspectrometer_plugin() {}
    public:
        ~adspectrometer_plugin() {}

        static std::shared_ptr< adspectrometer_plugin > instance_;

        static adspectrometer_plugin * instance() {
            static std::once_flag flag;
            std::call_once( flag, [] () {
                struct make_shared_enabler : public adspectrometer_plugin {};
                instance_ = std::make_shared< make_shared_enabler >();
            } );

            return instance_.get();
        }

        // plugin
        void * query_interface_workaround( const char * ) override { return 0; }

        void accept( adplugin::visitor&, const char * adplugin ) override;

        const char * iid() const override { return adspectrometer::names::iid_adspectrometer_plugin; }
    };

    std::shared_ptr< adspectrometer_plugin > adspectrometer_plugin::instance_;

}

extern "C" {
    DECL_EXPORT adplugin::plugin * adplugin_plugin_instance();
}

adplugin::plugin *
adplugin_plugin_instance()
{
    return adspectrometer::adspectrometer_plugin::instance();
}

///////////////////////////
using namespace adspectrometer;

void
adspectrometer_plugin::accept( adplugin::visitor& visitor, const char * adplugin )
{
    using adcontrols::massspectrometer_factory_type;
    static const boost::uuids::uuid clsid = boost::uuids::string_generator()( adspectrometer::MassSpectrometer::clsid_text );

    if ( auto factory =
         std::make_shared< massspectrometer_factory_type< MassSpectrometer > >( MassSpectrometer::class_name, clsid ) ) {
        
        adcontrols::MassSpectrometerBroker::register_factory( factory.get() );
    }
}
