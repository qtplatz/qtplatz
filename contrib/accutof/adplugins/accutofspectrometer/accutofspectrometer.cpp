/**************************************************************************
** Copyright (C) 2019 MS-Cheminformatics LLC
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
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
#include "datainterpreter.hpp"
#include "constants.hpp"
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/datainterpreterbroker.hpp>
#include <adcontrols/datainterpreter_factory.hpp>
#include <mutex>

namespace accutof { namespace spectrometer {

        class accutofspectrometer_plugin : public adplugin::plugin {

            accutofspectrometer_plugin( const accutofspectrometer_plugin& ) = delete;
#if _MSC_VER >= 1900
        public:
#endif
            accutofspectrometer_plugin() {}
        public:
            ~accutofspectrometer_plugin() {}

            static std::shared_ptr< accutofspectrometer_plugin > instance_;

            static accutofspectrometer_plugin * instance() {
                static std::once_flag flag;
                std::call_once( flag, [] () {
                                          struct make_shared_enabler : public accutofspectrometer_plugin {};
                                          instance_ = std::make_shared< make_shared_enabler >();
                                      } );

                return instance_.get();
            }

            // plugin
            void * query_interface_workaround( const char * ) override { return 0; }

            void accept( adplugin::visitor&, const char * adplugin ) override;

            const char * iid() const override { return names::iid_accutofspectrometer_plugin; }
        };

        std::shared_ptr< accutofspectrometer_plugin > accutofspectrometer_plugin::instance_;
    }
}

extern "C" {
    DECL_EXPORT adplugin::plugin * adplugin_plugin_instance();
}

adplugin::plugin *
adplugin_plugin_instance()
{
    return accutof::spectrometer::accutofspectrometer_plugin::instance();
}

using namespace accutof::spectrometer;

void
accutofspectrometer_plugin::accept( adplugin::visitor& visitor, const char * adplugin )
{
    if ( auto factory =
         std::make_shared< adcontrols::massspectrometer_factory_type< MassSpectrometer > >( names::objtext_massspectrometer, iids::uuid_massspectrometer ) ) {

        adcontrols::MassSpectrometerBroker::register_factory( factory.get() );
    }

    if ( auto factory =
         std::make_shared< adcontrols::datainterpreter::factory_type< accutofspectrometer::DataInterpreter > >() ) {

        adcontrols::DataInterpreterBroker::register_factory( factory, iids::uuid_datainterpreter, names::objtext_datainterpreter );
    }

}
