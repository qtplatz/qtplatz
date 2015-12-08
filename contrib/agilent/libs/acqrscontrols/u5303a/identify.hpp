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

#include "../acqrscontrols_global.hpp"
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <string>
#include <compiler/pragma_warning.hpp>

namespace boost { namespace serialization { class access; } }

namespace acqrscontrols {
    namespace u5303a {
        template<typename T> class identify_archive;

        class ACQRSCONTROLSSHARED_EXPORT identify {
        public:
            identify();
            identify( const identify& );

            std::string& Identifier();
            std::string& Revision();
            std::string& Vendor();
            std::string& Description();
            std::string& InstrumentModel();
            std::string& FirmwareRevision();
            std::string& SerialNumber();
            std::string& Options();
            std::string& IOVersion();
            uint32_t& NbrADCBits();

            const std::string& Identifier() const;
            const std::string& Revision() const;
            const std::string& Vendor() const;
            const std::string& Description() const;
            const std::string& InstrumentModel() const;
            const std::string& FirmwareRevision() const;
            const std::string& SerialNumber() const;
            const std::string& Options() const;
            const std::string& IOVersion() const;
            uint32_t NbrADCBits() const;

        private:
            pragma_diagnostic_ignore_4251
            std::string Identifier_;
            std::string Revision_;
            std::string Vendor_;
            std::string Description_;
            std::string InstrumentModel_;
            std::string FirmwareRevision_;
            std::string SerialNumber_;
            std::string Options_;
            std::string IOVersion_;
            pragma_diagnostic_pop

            uint32_t    NbrADCBits_;

            friend class identify_archive < identify > ;
            friend class identify_archive < const identify > ;
            friend class boost::serialization::access;
            template<class Archive>
            void serialize( Archive& ar, const unsigned int version );
        };

    }
}

BOOST_CLASS_VERSION( acqrscontrols::u5303a::identify, 1 )
