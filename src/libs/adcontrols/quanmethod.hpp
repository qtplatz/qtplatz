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

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT QuanMethod  {
    public:
        QuanMethod();

        enum CalibEq {
            idCalibOnePoint
            , idCalibLinear_origin
            , idCalibLinear
            , idCalibPolynomials
        };

        enum CalibWaiting {
            idWait_C1
            , idWait_C2
            , idWait_C3
            , idWait_Y1
            , idWait_Y2
            , idWait_Y3
        };

        CalibEq equation() const;
        void equation( CalibEq );

        bool isChromatogram() const;
        void isChromatogram( bool );

        bool isWaiting() const;
        void isWaiting( bool );
        
        CalibWaiting waiting() const;
        void waiting( CalibWaiting );

        bool ISTD() const;
        void ISTD( bool );

        uint32_t levels() const;
        void levels( uint32_t );

        uint32_t replicates() const;
        void replicates( uint32_t );

    private:
        
        CalibEq eq_;
        bool isChromatogram_;
        bool isISTD_;
        bool use_waiting_;
        CalibWaiting waiting_method_;
        uint32_t levels_;
        uint32_t replicates_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;

            ar & BOOST_SERIALIZATION_NVP(isChromatogram_)
                & BOOST_SERIALIZATION_NVP(eq_)
                & BOOST_SERIALIZATION_NVP(isISTD_)
                & BOOST_SERIALIZATION_NVP(use_waiting_)                
                & BOOST_SERIALIZATION_NVP(waiting_method_)
                & BOOST_SERIALIZATION_NVP(levels_)
                & BOOST_SERIALIZATION_NVP(replicates_)
                ;
        };

    };

}

BOOST_CLASS_VERSION( adcontrols::QuanMethod, 3 )

#endif // QUANMETHOD_HPP
