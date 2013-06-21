// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef TARGETINGMETHOD_H
#define TARGETINGMETHOD_H

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

#include <string>
#include <vector>
#include <compiler/disable_dll_interface.h>

namespace boost { namespace serialization {  class access;  } }

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT TargetingMethod {
    public:
        TargetingMethod();
        TargetingMethod( const TargetingMethod& );
        TargetingMethod& operator = ( const TargetingMethod& rhs );

        typedef std::pair< std::wstring, bool > value_type;

        std::vector< value_type >& adducts( bool positive = true );
        const std::vector< value_type >& adducts( bool positive = true ) const;

		std::pair< unsigned int, unsigned int > chargeState() const;
		void chargeState( unsigned int, unsigned int );

		std::vector< value_type >& formulae();
		const std::vector< value_type >& formulae() const;

    private:
        bool isPositiveIonMode_;
		// formula should be formatted of "<chemical-formula-string> -- any comment" | "numerical value -- any comment"
        std::vector< std::pair< std::wstring, bool > > formulae_;

		// adducts formula should be formatted of: "[+-]<formula-string>" where '+' or '-' specify adduct or loss
        std::vector< std::pair< std::wstring, bool > > adductsPos_;
        std::vector< std::pair< std::wstring, bool > > adductsNeg_;
        unsigned int chargeStateMin_;
        unsigned int chargeStateMax_;

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( isPositiveIonMode_ );
            ar & BOOST_SERIALIZATION_NVP( formulae_ );
            ar & BOOST_SERIALIZATION_NVP( adductsPos_ );
            ar & BOOST_SERIALIZATION_NVP( adductsNeg_ );
            ar & BOOST_SERIALIZATION_NVP( chargeStateMin_ );
            ar & BOOST_SERIALIZATION_NVP( chargeStateMax_ );
        }
    };

}

#endif // TARGETINGMETHOD_H
