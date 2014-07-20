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

#ifndef QUANCOMPOUND_HPP
#define QUANCOMPOUND_HPP

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <string>
#include <vector>
#include <cstdint>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT QuanCompound {
    public:
        ~QuanCompound();
        QuanCompound();
        QuanCompound( const QuanCompound& );

        const wchar_t * display_name() const;
        void displya_name( const wchar_t * );
        const wchar_t * formula() const;
        void formula( const wchar_t * );
        bool isISTD() const;
        void isISTD( bool );
        int32_t idISTD() const;
        void idISTD( int32_t );
        void levels( size_t );
        size_t levels() const;
        const double * amounts() const;
        void amounts( const double *, size_t size );

    private:
#  include <compiler/diagnostic_push.h>
#  include <compiler/disable_dll_interface.h>
        std::wstring display_name_;
        std::wstring formula_;
        std::vector< double > amounts_;  // added amounts[ level ]
#   include <compiler/diagnostic_pop.h>
        bool isISTD_;  // am I an internal standard?
        int32_t idISTD_; // index for internal standad (referenced from non-istd

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( display_name_ );
            ar & BOOST_SERIALIZATION_NVP( formula_ );
        }
    };

}

BOOST_CLASS_VERSION( adcontrols::QuanCompound, 1 )

#endif // QUANCOMPOUND_HPP
