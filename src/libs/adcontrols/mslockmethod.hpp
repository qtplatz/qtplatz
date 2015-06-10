// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#ifndef MSLOCKMETHOD_H
#define MSLOCKMETHOD_H

#pragma once

#include <compiler/disable_dll_interface.h>
#include "adcontrols_global.h"
#include "msfinder.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <string>

namespace adcontrols {

    enum idToleranceMethod : int;
    enum idFindAlgorithm : int;

    class ADCONTROLSSHARED_EXPORT MSLockMethod {
    public:
		~MSLockMethod(void);
		MSLockMethod(void);
        MSLockMethod(const MSLockMethod &);
        MSLockMethod & operator = (const MSLockMethod & rhs);

        bool enabled() const;
        void setEnabled( bool );

        idToleranceMethod toleranceMethod() const;
        void setToleranceMethod( idToleranceMethod );
        double tolerance( idToleranceMethod ) const;
        void setTolerance( idToleranceMethod, double );

        bool enablePeakThreshold() const;
        void setEnablePeakThreshold( bool );
        double peakIntensityThreshold() const;
        void setPeakIntensityThreshold( double );

        idFindAlgorithm algorithm() const;
        void setAlgorithm( idFindAlgorithm );

        void setReferences( const wchar_t * dataClass, const wchar_t * xml );
        const wchar_t * xmlDataClass() const;
        const wchar_t * xmlReferences() const;

    private:
        bool enabled_;
        bool enablePeakThreshold_;
        idToleranceMethod toleranceMethod_;
        idFindAlgorithm algorithm_;
        double toleranceDa_;
        double tolerancePpm_;
        double peakIntensityThreshold_;
        std::wstring xmlDataClass_;
        std::wstring xmlReferences_; // << lockmass::references

        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
            (void)version;
            ar & BOOST_SERIALIZATION_NVP(enabled_)
                & BOOST_SERIALIZATION_NVP(toleranceMethod_)
                & BOOST_SERIALIZATION_NVP(algorithm_)
                & BOOST_SERIALIZATION_NVP(toleranceDa_)
                & BOOST_SERIALIZATION_NVP(tolerancePpm_)
                & BOOST_SERIALIZATION_NVP(peakIntensityThreshold_)
                & BOOST_SERIALIZATION_NVP(xmlDataClass_)
                & BOOST_SERIALIZATION_NVP(xmlReferences_)
                ;
        }

    };

}

#endif // MSLOCKMETHOD_H
