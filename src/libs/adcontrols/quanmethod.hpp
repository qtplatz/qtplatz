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
#include "idaudit.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>
#include <string>
#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT QuanMethod {

    public:
        ~QuanMethod();
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

        const idAudit& ident() const { return ident_; }

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

        bool isInternalStandard() const;
        void isInternalStandard( bool );

        bool isExternalStandard() const { return !isInternalStandard(); }

        uint32_t levels() const;
        void levels( uint32_t );

        uint32_t replicates() const;
        void replicates( uint32_t );

        uint32_t debug_level() const;
        void debug_level( uint32_t );

        bool save_on_datasource() const;
        void save_on_datasource( bool );        

        const wchar_t * quanMethodFilename() const { return quanMethodFilename_.c_str(); }
        void quanMethodFilename( const wchar_t * d ) { quanMethodFilename_ = d; }
        const wchar_t * quanCompoundsFilename() const { return quanCompoundsFilename_.c_str(); }
        void quanCompoundsFilename( const wchar_t * d ) { quanCompoundsFilename_ = d; }
        const wchar_t * quanSequenceFilename() const { return quanSequenceFilename_.c_str(); }
         void quanSequenceFilename( const wchar_t * d ) { quanSequenceFilename_ = d; }

    private:
        idAudit ident_;
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
        uint32_t debug_level_; // determine which intermediate results to be stored on database
        bool save_on_datasource_;
        
        std::wstring quanMethodFilename_;
        std::wstring quanCompoundsFilename_;
        std::wstring quanSequenceFilename_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;

            ar & BOOST_SERIALIZATION_NVP( ident_ )
                & BOOST_SERIALIZATION_NVP( isChromatogram_ )
                & BOOST_SERIALIZATION_NVP( isISTD_ )
                & BOOST_SERIALIZATION_NVP( use_weighting_ )
                & BOOST_SERIALIZATION_NVP( use_bracketing_ )
                & BOOST_SERIALIZATION_NVP( eq_ )
                & BOOST_SERIALIZATION_NVP( weighting_ )
                & BOOST_SERIALIZATION_NVP( bracketing_ )
                & BOOST_SERIALIZATION_NVP( levels_ )
                & BOOST_SERIALIZATION_NVP( replicates_ )
                & BOOST_SERIALIZATION_NVP( polynomialOrder_ )
                & BOOST_SERIALIZATION_NVP( quanMethodFilename_ )
                & BOOST_SERIALIZATION_NVP( quanCompoundsFilename_ )
                & BOOST_SERIALIZATION_NVP( quanSequenceFilename_ )
                ;
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( debug_level_ );
                ar & BOOST_SERIALIZATION_NVP( save_on_datasource_ );
            }
        };

    };

}

BOOST_CLASS_VERSION( adcontrols::QuanMethod, 2 )

#endif // QUANMETHOD_HPP
