/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MSCHROMATOGRAMMETHOD_HPP
#define MSCHROMATOGRAMMETHOD_HPP

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/version.hpp>
#include <memory>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSChromatogramMethod {
    public:
        ~MSChromatogramMethod();
        MSChromatogramMethod();
        MSChromatogramMethod( const MSChromatogramMethod& );
        MSChromatogramMethod& operator = ( const MSChromatogramMethod& );

        static const wchar_t * dataClass() { return L"adcontrols::MSChromatogramMethod"; }

        enum DataSource { Profile, Centroid };
        enum WidthMethod { widthInDa, widthInRP };

        DataSource dataSource() const;
        void dataSource( enum DataSource );

        WidthMethod widthMethod() const;
        void widthMethod( WidthMethod );

        double width( WidthMethod method = widthInDa ) const;
        void width( double value, WidthMethod );

        double lower_limit() const;
        double upper_limit() const;
        void lower_limit( double );
        void upper_limit( double );
        double width_at_mass( double mass ) const;
        bool operator == ( const MSChromatogramMethod& ) const;
        
    private:

#   if  defined _MSC_VER
#   pragma warning(disable:4251)
#   endif

        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
    
}

BOOST_CLASS_VERSION( adcontrols::MSChromatogramMethod, 3 )

#endif // MSCHROMATOGRAMMETHOD_HPP
