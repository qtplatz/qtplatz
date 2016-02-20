// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <string>
#include <vector>
#include <memory>

namespace boost { namespace uuids { struct uuid; } }

namespace adcontrols {

    class MassSpectrometer;
	class massspectrometer_factory;

    class ADCONTROLSSHARED_EXPORT MassSpectrometerBroker {
        MassSpectrometerBroker(void);
        ~MassSpectrometerBroker(void);
    public:
        static bool register_factory( massspectrometer_factory* );
        
        static massspectrometer_factory* find_factory( const boost::uuids::uuid& );
        static massspectrometer_factory* find_factory( const std::string& objtext );

        // helper methods
        //static const boost::uuids::uuid name_to_uuid( const std::wstring& objext );
        //static const boost::uuids::uuid name_to_uuid( const std::string& objext );

        static std::shared_ptr< adcontrols::MassSpectrometer > make_massspectrometer( const boost::uuids::uuid& );

        // compatibility for the objects defined prior to v3.2.5
        static std::shared_ptr< adcontrols::MassSpectrometer > make_massspectrometer( const std::string& objtext );

        //
        static std::vector< std::pair< boost::uuids::uuid, std::string > > installed_uuids();

    private:
        class impl;
    };
    
}
