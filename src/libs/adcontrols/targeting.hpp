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
        struct ADCONTROLSSHARED_EXPORT Candidate;

        static const wchar_t * dataClass() { return L"adcontrols::Targeting"; }

        Targeting();
        Targeting( const Targeting& );
        Targeting( const TargetingMethod& );

        bool operator ()( const MassSpectrum& );
        bool operator ()( MassSpectrum& );
        bool force_find( const MassSpectrum&, const std::string& formula, int32_t fcn ); // call from quanchromatogramprocessor

        const std::vector< Candidate >& candidates() const { return candidates_; }

        struct isotope {
            int32_t idx;
            double mass;
            double abundance_ratio;
            double abundance_ratio_error;
            double exact_mass;
            double exact_abundance;
            isotope() : idx(-1), mass(0), abundance_ratio(0), abundance_ratio_error(0), exact_mass(0), exact_abundance(0) {}
            isotope( size_t _1, double _2, double _3, double _4, double _5, double _6 )
                : idx(_1), mass(_2), abundance_ratio(_3), abundance_ratio_error(_4), exact_mass(_5), exact_abundance(_6) {}
            isotope( const isotope& t )
                : idx( t.idx ), mass( t.mass ), abundance_ratio( t.abundance_ratio ), abundance_ratio_error( t.abundance_ratio_error )
                , exact_mass( t.exact_mass ), exact_abundance(t.exact_abundance) {}
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, unsigned int ) {
                ar & BOOST_SERIALIZATION_NVP( idx );
                ar & BOOST_SERIALIZATION_NVP( mass );
                ar & BOOST_SERIALIZATION_NVP( abundance_ratio );
                ar & BOOST_SERIALIZATION_NVP( abundance_ratio_error );
                ar & BOOST_SERIALIZATION_NVP( exact_mass );
                ar & BOOST_SERIALIZATION_NVP( exact_abundance );
            }
        };

        struct Candidate {
            uint32_t idx; // peak index on mass-spectrum
            uint32_t fcn; // protocol (aka segment) id
            int32_t charge;
            double mass;  // This used to an error from exact mass, change it to found mass at V2 (2019-AUG-15)
            std::string formula; // this is the exact formula matched with the peak (contains adducts)
            double exact_mass;               // V2
            int32_t score;                   // V2
            std::vector< isotope > isotopes; // V2
            Candidate();
            Candidate( const Candidate& );
            Candidate( uint32_t idx, uint32_t fcn, int32_t charge, double mass, double exact_mass, const std::string& formula );
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, unsigned int version ) {
                ar & BOOST_SERIALIZATION_NVP( idx );
                ar & BOOST_SERIALIZATION_NVP( fcn );
                ar & BOOST_SERIALIZATION_NVP( charge );
                ar & BOOST_SERIALIZATION_NVP( mass );
                ar & BOOST_SERIALIZATION_NVP( formula );
                if ( version >= 2 ) {
                    ar & BOOST_SERIALIZATION_NVP( exact_mass );
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
