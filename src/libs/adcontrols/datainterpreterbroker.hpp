// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@MS-Cheminformatics.com
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

#include "adcontrols_global.h"
#include "datainterpreter.hpp"
#include <memory>

namespace boost { namespace uuids { struct uuid; } };

namespace adcontrols {

    class DataInterpreterFactory;

    // This class supports v2 (rawdata_v2) access;
    // (v3 access may use DataReader singleton interface directory)

    class ADCONTROLSSHARED_EXPORT DataInterpreterBroker {
        DataInterpreterBroker();
        ~DataInterpreterBroker();
    public:
        static bool register_factory( std::shared_ptr< DataInterpreterFactory >, const boost::uuids::uuid&, const std::string& dataInterpreterClsid );

        static DataInterpreterFactory * find_factory( const std::string& dataInterpreterClsid );
        static DataInterpreterFactory * find_factory( const boost::uuids::uuid& );
        static std::shared_ptr< adcontrols::DataInterpreter > make_datainterpreter( const std::string& dataInterpreterClsid );
        static std::shared_ptr< adcontrols::DataInterpreter > make_datainterpreter( const boost::uuids::uuid& );
        static std::vector< std::pair< boost::uuids::uuid, std::string > > installed_uuids();

        static boost::uuids::uuid name_to_uuid( const std::string& dataInterpreterClsid );

    private:
        class impl;
    };
    
}
