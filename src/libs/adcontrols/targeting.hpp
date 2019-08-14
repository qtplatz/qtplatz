/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef TARGETING_HPP
#define TARGETING_HPP

#include "adcontrols_global.h"
#include <string>
#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <tuple>
#include <memory>

namespace adcontrols {

    class TargetingMethod;
    class MassSpectrum;
    class ADCONTROLSSHARED_EXPORT Targeting;

    class Targeting {
    public:
        struct Candidate;

        static const wchar_t * dataClass() { return L"adcontrols::Targeting"; }

        Targeting();
        Targeting( const Targeting& );
        Targeting( const TargetingMethod& );

        bool operator ()( const MassSpectrum& );
        bool operator ()( MassSpectrum& );
        bool force_find( const MassSpectrum&, const std::string& formula, int32_t fcn ); // call from quanchromatogramprocessor

        const std::vector< Candidate >& candidates() const { return candidates_; }

        struct Candidate {
            uint32_t idx; // peak index on mass-spectrum
            uint32_t fcn; // protocol (aka segment) id
            uint32_t charge;
            double mass_error;
            std::string formula; // this is the exact formula matched with the peak (contains adducts)
            int32_t score;
            std::vector< std::pair< uint32_t, uint32_t > > isotopes; // pair of index,fcn (protocol)
            Candidate();
            Candidate( const Candidate& );
            Candidate( uint32_t idx, uint32_t fcn, uint32_t charge, double mass_error, const std::string& formula );
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, unsigned int version ) {
                ar & BOOST_SERIALIZATION_NVP( idx )
                    & BOOST_SERIALIZATION_NVP( fcn )
                    & BOOST_SERIALIZATION_NVP( charge )
                    & BOOST_SERIALIZATION_NVP( mass_error )
                    & BOOST_SERIALIZATION_NVP( formula )
                    ;
                if ( version >= 2 ) {
                    ar & BOOST_SERIALIZATION_NVP( score );
                    ar & BOOST_SERIALIZATION_NVP( isotopes );
                }
            }
        };

        // 'formula+adduct', exact mass, charge
        static std::vector< std::tuple<std::string, double, int> >
        make_mapping( const std::pair<uint32_t, uint32_t>&, const std::string& formula, const std::string& adducts, bool positive_polairy );

        static bool archive( std::ostream&, const Targeting& );
        static bool restore( std::istream&, Targeting& );

    private:
        std::shared_ptr< TargetingMethod > method_;
        std::vector< Candidate > candidates_;
        typedef std::pair< double, std::string > adduct_type;

        std::vector< std::pair< std::string, double > > active_formula_;

        void setup( const TargetingMethod& );
        bool find_candidate( const MassSpectrum& ms, int fcn, bool polarity_positive ); //, const std::vector< charge_adduct_type >& list );

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( candidates_ )
                ;
        }
    };

    typedef std::shared_ptr< Targeting > TargetingPtr;

}

BOOST_CLASS_VERSION( adcontrols::Targeting::Candidate, 2 )

#endif // TARGETING_HPP
