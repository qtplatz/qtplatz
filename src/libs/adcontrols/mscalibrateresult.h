// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include <boost/smart_ptr.hpp>
#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/version.hpp>


namespace adcontrols {

    class MSReferences;
    class MSCalibration;

    class ADCONTROLSSHARED_EXPORT MSCalibrateResult {
    public:
        ~MSCalibrateResult();
        MSCalibrateResult();
        MSCalibrateResult( const MSCalibrateResult & t );

        const MSReferences& references() const;
        MSReferences& references();
        void references( const MSReferences& );

        const MSCalibration& calibration() const;
        MSCalibration& calibration();
        void calibration( const MSCalibration& );

    private:

#pragma warning( disable:4251 )
        boost::scoped_ptr< MSReferences > references_;
        boost::scoped_ptr< MSCalibration > calibration_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            using namespace boost::serialization;
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP(references_);
                ar & BOOST_SERIALIZATION_NVP(calibration_);
            }
        }

    };

   typedef boost::shared_ptr<MSCalibrateResult> MSCalibrateResultPtr;

}


