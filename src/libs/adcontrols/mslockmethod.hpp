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
#include <boost/serialization/version.hpp>
#include <string>
#include <memory>

namespace boost {
    namespace serialization { class access; }
}

namespace adcontrols {

    enum idToleranceMethod : int;
    enum idFindAlgorithm : int;
    class moltable;

    template< typename T > class MSLockMethod_archive;
    
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

        const moltable& molecules() const;
        moltable& molecules();
        void setMolecules( const moltable& );

    private:
        bool enabled_;
        bool enablePeakThreshold_;
        idToleranceMethod toleranceMethod_;
        idFindAlgorithm algorithm_;
        double toleranceDa_;
        double tolerancePpm_;
        double peakIntensityThreshold_;
        std::unique_ptr< moltable > molecules_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
        friend class MSLockMethod_archive< MSLockMethod >;
        friend class MSLockMethod_archive< const MSLockMethod >;
    };

}

BOOST_CLASS_VERSION( adcontrols::MSLockMethod, 1 )

#endif // MSLOCKMETHOD_H
