// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <boost/smart_ptr.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/version.hpp>

namespace adcontrols {

    class MSReferenceDefns;

    class ADCONTROLSSHARED_EXPORT MSCalibrateMethod {
    public:
        ~MSCalibrateMethod();
        MSCalibrateMethod();
        MSCalibrateMethod( const MSCalibrateMethod& );
        MSCalibrateMethod& operator = ( const MSCalibrateMethod& );

        unsigned int polynomialDegree() const;
        void polynomialDegree( unsigned int );
        double massToleranceDa() const;
        void massToleranceDa( double );
        double minimumRAPercent() const;
        void minimumRAPercent( double );
        double lowMass() const;
        void lowMass( double );
        double highMass() const;
        void highMass( double );
        
        const MSReferenceDefns& refDefns();
        void refDefns( const MSReferenceDefns& );

    private:
        unsigned int polynomialDegree_;
        double massToleranceDa_;
        double minimumRAPercent_;
        double lowMass_;
        double highMass_;
# pragma warning(disable:4251)
        boost::scoped_ptr<MSReferenceDefns> refDefns_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP(polynomialDegree_);
                ar & BOOST_SERIALIZATION_NVP(massToleranceDa_);
                ar & BOOST_SERIALIZATION_NVP(minimumRAPercent_);
                ar & BOOST_SERIALIZATION_NVP(lowMass_);
                ar & BOOST_SERIALIZATION_NVP(highMass_);
                ar & BOOST_SERIALIZATION_NVP(refDefns_);
            }
       }

    };
  
}


