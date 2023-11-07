// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC
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
#include <adcontrols/constants_fwd.hpp>
#include <boost/serialization/version.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace boost { namespace serialization {  class access;  } }

namespace adcontrols {

    enum idFindAlgorithm : int;        // defined in msfinder.hpp
    enum idToleranceMethod : int;      // defined in msfinder.hpp

    class ADCONTROLSSHARED_EXPORT MetIdMethod;

    class MetIdMethod {
    public:
        ~MetIdMethod();
        MetIdMethod();
        MetIdMethod( const MetIdMethod& );
        MetIdMethod& operator = ( const MetIdMethod& rhs );

        adcontrols::ion_polarity polarity() const;
        void setPolarity( adcontrols::ion_polarity );
        [[deprecated]] bool isPositiveMode() const;
        [[deprecated]] void setPositiveMode( bool );

        MetIdMethod& operator << ( std::pair< bool, std::string >&& );
        void setAdducts( const std::vector< std::pair< bool, std::string > >& );
        const std::vector< std::pair< bool, std::string > >& adducts() const;
        std::vector< std::pair< bool, std::string > >& adducts();

		std::pair< uint32_t, uint32_t > chargeState() const;
		void chargeState( std::pair< uint32_t, uint32_t >&& );

        idToleranceMethod toleranceMethod() const;
        void setToleranceMethod( idToleranceMethod );

        idFindAlgorithm findAlgorithm() const;
        void setFindAlgorithm( idFindAlgorithm );

        double tolerance() const;
        double tolerance( idToleranceMethod ) const;
        void setTolerance( idToleranceMethod, double );

    private:
        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version );
        friend ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const MetIdMethod& );
        friend ADCONTROLSSHARED_EXPORT MetIdMethod
        tag_invoke( const boost::json::value_to_tag< MetIdMethod >&, const boost::json::value& jv );
    };

    ADCONTROLSSHARED_EXPORT
    void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const MetIdMethod& );

    ADCONTROLSSHARED_EXPORT
    MetIdMethod tag_invoke( const boost::json::value_to_tag< MetIdMethod >&, const boost::json::value& jv );
}

// Archive version 5 and later is using 'impl' idiom
BOOST_CLASS_VERSION( adcontrols::MetIdMethod, 0 )
