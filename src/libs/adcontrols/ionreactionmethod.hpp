// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include "adcontrols_global.h"
#include "constants_fwd.hpp"
#include "msfinder.hpp"
#include <boost/serialization/version.hpp>
#include <adportable/json/extract.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace boost { namespace serialization {  class access;  } }

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT IonReactionMethod;

    class IonReactionMethod {
    public:
        IonReactionMethod();
        IonReactionMethod( const IonReactionMethod& );
        IonReactionMethod& operator = ( const IonReactionMethod& rhs );

        std::vector< std::pair< bool, std::string > >& addlose( ion_polarity );
        const std::vector< std::pair< bool, std::string > >& addlose( ion_polarity ) const;

		std::pair< uint32_t, uint32_t > chargeState( ion_polarity ) const;
		void chargeState( std::pair< uint32_t, uint32_t >&&, ion_polarity );

        ion_polarity polarity() const;
        void set_polarity( ion_polarity ) const;
        const std::string& i8n() const;
        void set_i8n( std::string&& );
        const std::string& description() const;
        void set_description( std::string&& );

    private:
        class impl;
        impl * impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version );
        friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const IonReactionMethod& );
        friend ADCONTROLSSHARED_EXPORT IonReactionMethod
        tag_invoke( boost::json::value_to_tag< IonReactionMethod >&, const boost::json::value& jv );

    };

    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const IonReactionMethod& );

    ADCONTROLSSHARED_EXPORT
    IonReactionMethod tag_invoke( boost::json::value_to_tag< IonReactionMethod >&, const boost::json::value& jv );

}

BOOST_CLASS_VERSION( adcontrols::IonReactionMethod, 1 )
