/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometer_factory.hpp>
#include <adplugin/plugin.hpp>
#include <memory>
#include <atomic>
#include <mutex>
#include "../acqrscontrols_global.hpp"

namespace acqrscontrols {
    namespace ap240 {

        class ScanLaw;
        class DataInterpreter;

        class ACQRSCONTROLSSHARED_EXPORT MassSpectrometer : public adcontrols::MassSpectrometer
            , public adcontrols::massspectrometer_factory
            , public adplugin::plugin {

            MassSpectrometer();
        public:
            ~MassSpectrometer();
            MassSpectrometer( adcontrols::datafile * );

            static MassSpectrometer * instance();
            static void dispose();

            // adcontrols::MassSpectrometer
            const adcontrols::ScanLaw& getScanLaw() const override;
            std::shared_ptr< adcontrols::ScanLaw > scanLaw( const adcontrols::MSProperty& ) const override;
            const adcontrols::DataInterpreter& getDataInterpreter() const override;

            // massspectrometer_factory
            const wchar_t * name() const override;
            adcontrols::MassSpectrometer * get( const wchar_t * ) override;
            std::shared_ptr< adcontrols::MassSpectrometer > create( const wchar_t *, adcontrols::datafile * ) const;

            // plugin
            const char * iid() const;
            void accept( adplugin::visitor&, const char * pluginspecs );
            virtual void * query_interface_workaround( const char * typenam );

        private:
            static std::mutex mutex_;
            static std::atomic< MassSpectrometer * > instance_;
            std::shared_ptr< ScanLaw > scanlaw_;
            std::shared_ptr< DataInterpreter > interpreter_;
        };

    }
}