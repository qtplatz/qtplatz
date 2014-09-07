// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include <boost/serialization/version.hpp>
#include <memory>

namespace boost { 
    namespace uuids { struct uuid; }
    namespace serialization { class access; }
}

namespace adcontrols {

    class MSReferences;
    class MSCalibration;
    class MSAssignedMasses;

    class ADCONTROLSSHARED_EXPORT MSCalibrateResult {
    public:
        ~MSCalibrateResult();
        MSCalibrateResult();
        MSCalibrateResult( const MSCalibrateResult & t );
        static const wchar_t * dataClass() { return L"MSCalibrateResult"; }
        const MSCalibrateResult& operator = ( const MSCalibrateResult & t );

        double threshold() const;
        void threshold( double );

        double tolerance() const;
        void tolerance( double );

        const MSReferences& references() const;
        MSReferences& references();
        void references( const MSReferences& );

        const MSAssignedMasses& assignedMasses() const;
        MSAssignedMasses& assignedMasses();
        void assignedMasses( const MSAssignedMasses& );

        const MSCalibration& calibration() const;
        MSCalibration& calibration();
        void calibration( const MSCalibration& );

        int mode() const;
        void mode( int );
        const wchar_t * description() const;
        void description( const wchar_t * );

    private:

# if defined _MSC_VER
#  pragma warning( disable:4251 )
# endif

        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive>  void serialize( Archive& ar, const unsigned int version );

    public:
        static bool archive( std::ostream&, const MSCalibrateResult& );
        static bool restore( std::istream&, MSCalibrateResult& );
        static bool xml_archive( std::wostream&, const MSCalibrateResult& );
        static bool xml_restore( std::wistream&, MSCalibrateResult& );
    };

   typedef std::shared_ptr<MSCalibrateResult> MSCalibrateResultPtr;

}

BOOST_CLASS_VERSION( adcontrols::MSCalibrateResult, 3 )
