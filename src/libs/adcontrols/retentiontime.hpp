// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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
#include <boost/serialization/utility.hpp>
#include <utility>
#include <cstdint>
#include <vector>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT RetentionTime {
    public:
        enum algo { Maxima, ParaboraFitting, Moment };
        RetentionTime();
        RetentionTime( const RetentionTime& t );

        void setAlgorithm( algo );
        void setThreshold( double, double );
        void setBoundary( double, double );
        void setEq( double a, double b, double c );

        algo algorithm() const;
        double threshold( int ) const;
        double boundary( int ) const;
        bool eq( double& a, double& b, double& c ) const;

    private:
#if defined _MSC_VER
# pragma warning(disable:4251)
#endif
        algo algo_;
        std::pair< double, double > threshold_;
        std::pair< double, double > boundary_;
        std::vector< double > eq_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( algo_ );
            ar & BOOST_SERIALIZATION_NVP( threshold_ );
            ar & BOOST_SERIALIZATION_NVP( boundary_ );
            ar & BOOST_SERIALIZATION_NVP( eq_ );
        }
    };
}

