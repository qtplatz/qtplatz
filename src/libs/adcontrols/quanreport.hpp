/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANREPORT_HPP
#define QUANREPORT_HPP

#include "adcontrols_global.h"
#include <compiler/disable_dll_interface.h>
//#include "quansequence.hpp"
//#include "quancompounds.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace adcontrols {

    class QuanCompounds;
    class QuanSequence;

    class ADCONTROLSSHARED_EXPORT QuanReport {
    public:
        ~QuanReport();
        QuanReport();
        QuanReport( const QuanReport& );
    private:
        boost::uuids::uuid uuid_;

        std::string processDate_;
        std::string terminalId_;
        std::string operatorId_;
        std::wstring operatorName_;
        std::shared_ptr< QuanCompounds > compounds_;
        std::shared_ptr< QuanSequence > sequence_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( uuid_ )
                & BOOST_SERIALIZATION_NVP( processDate_ )
                & BOOST_SERIALIZATION_NVP( terminalId_ )
                & BOOST_SERIALIZATION_NVP( operatorId_ )
                & BOOST_SERIALIZATION_NVP( operatorName_ )
                & BOOST_SERIALIZATION_NVP( compounds_ )
                ;
        }
        
    };

}

BOOST_CLASS_VERSION( adcontrols::QuanReport, 1 )


#endif // QUANREPORT_HPP
