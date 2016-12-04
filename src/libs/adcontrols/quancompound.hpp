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
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>

namespace boost { 
    namespace uuids { struct uuid; }
    namespace serialization { class access; }
}

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT QuanCompound {
        QuanCompound( const boost::uuids::uuid& );
    public:
        ~QuanCompound();
        QuanCompound();
        QuanCompound( const QuanCompound& );
        QuanCompound& operator = ( const QuanCompound& );

        static QuanCompound null(); // null compound reference for not identifid;

        const boost::uuids::uuid& uuid() const;
        uint32_t row() const;
        void row( uint32_t );
        
        const wchar_t * display_name() const;
        void displya_name( const wchar_t * );
        const char * formula() const;
        void formula( const char * );
        bool isLKMSRef() const;
        void isLKMSRef( bool );
        bool isTimeRef() const;
        void isTimeRef( bool );
        bool isISTD() const;
        void isISTD( bool );
        int32_t idISTD() const;
        void idISTD( int32_t );
        void levels( size_t );
        size_t levels() const;
        const double * amounts() const;
        void amounts( const double *, size_t size );
        double mass() const;
        void mass( double v );
        double tR() const;
        void tR( double v );
        const wchar_t * description() const;
        void description( const wchar_t * );
        double criteria( bool second = false ) const;
        void criteria( double v, bool second = false );

    private:

#   if  defined _MSC_VER
#   pragma warning(disable:4251)
#   endif
        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int );
    };

}

BOOST_CLASS_VERSION( adcontrols::QuanCompound, 2 )

#endif // QUANCOMPOUND_HPP
