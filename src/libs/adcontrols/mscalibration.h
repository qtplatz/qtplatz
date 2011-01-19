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

#include <vector>
#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSCalibration {
    public:
        MSCalibration();
        MSCalibration( const MSCalibration& );
        MSCalibration( const std::vector<double>& );

        const std::string& date() const;
        void date( const std::string& );

        const std::wstring& calibId() const;
        void calibId( const std::wstring& );

        const std::vector< double >& coeffs() const;
        void coeffs( const std::vector<double>& );

        static double compute( const std::vector<double>&, double time );
        
    private:
        std::string calibDate_;
        std::wstring calibId_;
#pragma warning (disable:4251)
        std::vector< double > coeffs_;
//#pragma warning (default:4251)

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP(calibDate_);
                ar & BOOST_SERIALIZATION_NVP(calibId_);
                ar & BOOST_SERIALIZATION_NVP(coeffs_);
            }
        }
    };

}

