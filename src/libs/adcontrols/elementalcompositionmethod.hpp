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
#include <string>
#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

namespace boost { namespace serialization {
    class access;
} }

#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable: 4251 ) // dll-linkage for
#endif

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT ElementalCompositionMethod {
    public:
        ElementalCompositionMethod();

        enum ElectronMode { Even, Odd, OddEven };

        struct ADCONTROLSSHARED_EXPORT CompositionConstraint {
            std::string atom;
            size_t numMinimum;
            size_t numMaximum;
            CompositionConstraint( const std::string& atom = ""
                                   , size_t minimum = 0, size_t maximum = 0 );
            CompositionConstraint( const CompositionConstraint& );
        private:
            friend class boost::serialization::access;
            template<class Archive> void serialize(Archive& ar, const unsigned int version ) {
                using namespace boost::serialization;
                (void)version;
                ar & BOOST_SERIALIZATION_NVP( atom )
                    & BOOST_SERIALIZATION_NVP( numMinimum )
                    & BOOST_SERIALIZATION_NVP( numMaximum );
            }
        };

        ElectronMode electronMode() const;
        void electronMode( ElectronMode );
        bool toleranceInPpm() const;
        void toleranceInPpm( bool );
        double tolerance( bool ppm ) const;
        void tolerance( bool ppm, double );
        double dbeMinimum() const;
        void dbeMinimum( double );
        double dbeMaximum() const;
        void dbeMaximum( double );
        size_t numResults() const;
        void numResults( size_t );

        typedef std::vector< CompositionConstraint > vector_type;

        size_t size() const;
        vector_type::iterator begin();
        vector_type::iterator end();
        vector_type::const_iterator begin() const;
        vector_type::const_iterator end() const;
        void addCompositionConstraint( const CompositionConstraint& );
        vector_type::iterator erase( vector_type::iterator beg, vector_type::iterator end );

    private:
        ElectronMode electron_mode_;
        bool tolerance_in_ppm_;
        double tolerance_mDa_;
        double tolerance_ppm_;
        double dbe_minimum_;
        double dbe_maximum_;
        size_t numResults_;
#ifdef _MSC_VER
# pragma warning( push )
# pragma warning( disable: 4251 ) // dll-linkage for
#endif
        std::vector< CompositionConstraint > vec_;
#ifdef _MSC_VER
# pragma warning( pop )
#endif
    private:
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
            (void)version;
            ar & BOOST_SERIALIZATION_NVP( electron_mode_ )
                & BOOST_SERIALIZATION_NVP( tolerance_in_ppm_ )
                & BOOST_SERIALIZATION_NVP( tolerance_mDa_ )
                & BOOST_SERIALIZATION_NVP( tolerance_ppm_ )
                & BOOST_SERIALIZATION_NVP( dbe_minimum_ )
                & BOOST_SERIALIZATION_NVP( dbe_maximum_ )
                & BOOST_SERIALIZATION_NVP( numResults_ )
                & BOOST_SERIALIZATION_NVP( vec_ )
                ;
        }
    };

}

#ifdef _MSC_VER
# pragma warning( pop )
#endif