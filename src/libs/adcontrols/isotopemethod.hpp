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

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <string>
#include <vector>

#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable: 4251 ) // dll-linkage for
#endif

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT IsotopeMethod {
    public:
        ~IsotopeMethod();
        IsotopeMethod();
        IsotopeMethod( const IsotopeMethod& );

        IsotopeMethod & operator = (const IsotopeMethod & rhs);

        struct ADCONTROLSSHARED_EXPORT Formula {
        public:
			std::wstring description;  // name of structure, mol file name etc.
            std::wstring formula;
            std::wstring adduct;
            size_t chargeState;
            double relativeAmounts;
            bool positive;
            Formula();
            Formula( const Formula& );
			Formula( const std::wstring& desc
				     , const std::wstring& formula
                     , const std::wstring& adduct
                     , size_t chargeState
                     , double relativeAmounts
                     , bool positive = true );
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive& ar, const unsigned int version) {
                using namespace boost::serialization;
                (void)version;
				ar & BOOST_SERIALIZATION_NVP(description);
                ar & BOOST_SERIALIZATION_NVP(formula);
                ar & BOOST_SERIALIZATION_NVP(adduct);
                ar & BOOST_SERIALIZATION_NVP(chargeState);
                ar & BOOST_SERIALIZATION_NVP(relativeAmounts);
                ar & BOOST_SERIALIZATION_NVP(positive);
            }
        };

    public:
        typedef std::vector< Formula > vector_type;

        void clear();
        size_t size() const;
        void addFormula( const Formula& );

        vector_type::iterator begin();
        vector_type::iterator end();
        vector_type::const_iterator begin() const;
        vector_type::const_iterator end() const;
        inline vector_type::iterator erase( vector_type::iterator beg, vector_type::iterator end ) {
            return formulae_.erase( beg, end );
        }
 
        bool polarityPositive() const;
        void polarityPositive( bool );

        bool useElectronMass() const;
        void useElectronMass( bool );

        double threshold() const;
        void threshold( double );

        double resolution() const;
        void resolution( double );

    private:
        bool polarityPositive_;
        bool useElectronMass_;
        double	threshold_;		// %RA
        double	resolution_;	// Da

        std::vector< Formula > formulae_;  // formula, adduct

        // serialization
        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
            (void)version;
            ar & BOOST_SERIALIZATION_NVP(polarityPositive_);
            ar & BOOST_SERIALIZATION_NVP(useElectronMass_);
            ar & BOOST_SERIALIZATION_NVP(threshold_);
            ar & BOOST_SERIALIZATION_NVP(resolution_);
            ar & BOOST_SERIALIZATION_NVP(formulae_);
       }

    };

}

#ifdef _MSC_VER
# pragma warning( pop )
#endif