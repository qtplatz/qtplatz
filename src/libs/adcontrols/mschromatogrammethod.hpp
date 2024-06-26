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
#include <string>
#include <vector>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class moltable;
    class ADCONTROLSSHARED_EXPORT MSChromatogramMethod;

    class MSChromatogramMethod {
    public:
        ~MSChromatogramMethod();
        MSChromatogramMethod();
        MSChromatogramMethod( const MSChromatogramMethod& );
        MSChromatogramMethod& operator = ( const MSChromatogramMethod& );

        static const wchar_t * dataClass() { return L"adcontrols::MSChromatogramMethod"; }

        enum WidthMethod { widthInDa, widthInRP, widthPeakFWHM, widthTime };

        const std::string& dataReader() const;
        void setDataReader( const std::string& );

        WidthMethod widthMethod() const;
        void widthMethod( WidthMethod );

        double width( WidthMethod method = widthInDa ) const;
        void width( double value, WidthMethod );

        // deprecated
        double lower_limit() const;
        double upper_limit() const;
        void lower_limit( double );
        void upper_limit( double );
        // end deprecated

        double width_at_mass( double mass ) const;

        bool lockmass() const;
        void setLockmass( bool );

        double tolerance() const;
        void setTolerance( double );

        // auto m/z find from spectrum by tR
        bool enableAutoTargeting() const;
        void setEnableAutoTargeting( bool );

        double peakWidthForChromatogram() const;
        void setPeakWidthForChromatogram( double );

        bool operator == ( const MSChromatogramMethod& ) const;

        const moltable& molecules() const;
        moltable& molecules();
        void setMolecules( const moltable& );

    private:

        class impl;
        impl * impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

}

BOOST_CLASS_VERSION( adcontrols::MSChromatogramMethod, 4 )

#endif // MSCHROMATOGRAMMETHOD_HPP
