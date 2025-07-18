/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC, Toin, Mie Japan
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

namespace mzml {

    class datareader_factory : public adplugin::plugin {

        datareader_factory( const datareader_factory& ) = delete;
        const datareader_factory& operator = ( const datareader_factory& ) = delete;

        datareader_factory();
    public:
        ~datareader_factory();

        static adplugin::plugin * instance();

        void accept( adplugin::visitor&, const char * adplugin ) override;
        const char * iid() const override;
        void * query_interface_workaround( const char * typname ) override;

    private:
        static std::shared_ptr< adplugin::plugin > instance_;
    };

    struct IID_DataInterpreter {
        static const std::string value;
    };

    struct IID_DataReader {
        static const std::string value;
    };

}
