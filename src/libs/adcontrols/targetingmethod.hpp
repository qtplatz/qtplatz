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

        bool is_use_resolving_power() const;
        void is_use_resolving_power( bool );

        double resolving_power() const;
        void resolving_power( double );

        double peak_width() const;
        void peak_width( double );

        std::pair< bool, bool > isMassLimitsEnabled() const;
        void isLowMassLimitEnabled( bool );
        void isHighMassLimitEnabled( bool );
        
        double lowMassLimit() const;
        void lowMassLimit( double );

        double highMassLimit() const;
        void highMassLimit( double );

        double tolerance() const;
        void tolerance( double );

    private:
        bool isPositiveIonMode_;

        bool is_use_resolving_power_;
        double resolving_power_;
        double peak_width_;
        unsigned int chargeStateMin_;
        unsigned int chargeStateMax_;
        bool isLowMassLimitEnabled_;
        bool isHighMassLimitEnabled_;
        double lowMassLimit_;
        double highMassLimit_;
        double tolerance_;

		// formula should be formatted of "<chemical-formula-string> -- any comment" | "numerical value -- any comment"
        std::vector< std::pair< std::wstring, bool > > formulae_;

		// adducts formula should be formatted of: "[+-]<formula-string>" where '+' or '-' specify adduct or loss
        std::vector< std::pair< std::wstring, bool > > adductsPos_;
        std::vector< std::pair< std::wstring, bool > > adductsNeg_;

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( isPositiveIonMode_ );
            ar & BOOST_SERIALIZATION_NVP( is_use_resolving_power_ );
            ar & BOOST_SERIALIZATION_NVP( resolving_power_ );
            ar & BOOST_SERIALIZATION_NVP( peak_width_ );
            ar & BOOST_SERIALIZATION_NVP( chargeStateMin_ );
            ar & BOOST_SERIALIZATION_NVP( chargeStateMax_ );
            ar & BOOST_SERIALIZATION_NVP( isLowMassLimitEnabled_ );
            ar & BOOST_SERIALIZATION_NVP( isHighMassLimitEnabled_ );
            ar & BOOST_SERIALIZATION_NVP( lowMassLimit_ );
            ar & BOOST_SERIALIZATION_NVP( highMassLimit_ );
            ar & BOOST_SERIALIZATION_NVP( tolerance_ );
            ar & BOOST_SERIALIZATION_NVP( formulae_ );
            ar & BOOST_SERIALIZATION_NVP( adductsPos_ );
            ar & BOOST_SERIALIZATION_NVP( adductsNeg_ );
        }
    };

}

#endif // TARGETINGMETHOD_H
