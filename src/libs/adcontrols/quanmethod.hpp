/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef QUANMETHOD_HPP
#define QUANMETHOD_HPP

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>


namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT QuanMethod {

    public:
        QuanMethod();
        QuanMethod( const QuanMethod& );

        enum CalibEq {
            idCalibOnePoint
            , idCalibLinear_origin
            , idCalibLinear
            , idCalibPolynomials
        };

        enum CalibWeighting {
            idWeight_C1
            , idWeight_C2
            , idWeight_C3
            , idWeight_Y1
            , idWeight_Y2
            , idWeight_Y3
        };

        enum Bracketing {
            idBracketNone
            , idBracketStandard
            , idBracketOverlapped
            , idBracketAverage
        };

        CalibEq equation() const;
        void equation( CalibEq );
        
        uint32_t polynomialOrder() const;
        void polynomialOrder( uint32_t );

        bool isChromatogram() const;
        void isChromatogram( bool );

        bool isWeighting() const;
        void isWeighting( bool );

        bool isBracketing() const;
        void isBracketing( bool );

        Bracketing bracketing() const;
        void bracketing( Bracketing );
        
        CalibWeighting weighting() const;
        void weighting( CalibWeighting );

        bool ISTD() const;
        void ISTD( bool );

        uint32_t levels() const;
        void levels( uint32_t );

        uint32_t replicates() const;
        void replicates( uint32_t );

    private:
        
        bool isChromatogram_;
        bool isISTD_;
        bool use_weighting_;
        bool use_bracketing_;
        CalibEq eq_;
        CalibWeighting weighting_;
        Bracketing bracketing_;
        uint32_t levels_;
        uint32_t replicates_;
        uint32_t polynomialOrder_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;

            ar & BOOST_SERIALIZATION_NVP( isChromatogram_ )
                & BOOST_SERIALIZATION_NVP( isISTD_ )
                & BOOST_SERIALIZATION_NVP( use_weighting_ )
                & BOOST_SERIALIZATION_NVP( use_bracketing_ )
                & BOOST_SERIALIZATION_NVP( eq_ )
                & BOOST_SERIALIZATION_NVP( weighting_ )
                & BOOST_SERIALIZATION_NVP( bracketing_ )
                & BOOST_SERIALIZATION_NVP( levels_ )
                & BOOST_SERIALIZATION_NVP( replicates_ )
                & BOOST_SERIALIZATION_NVP( polynomialOrder_ )
                ;
        };

    };

}

BOOST_CLASS_VERSION( adcontrols::QuanMethod, 1 )

#endif // QUANMETHOD_HPP
