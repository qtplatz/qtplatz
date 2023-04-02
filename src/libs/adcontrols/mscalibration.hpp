// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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

#pragma once

#include "adcontrols_global.h"
#include <cstdint>
#include <vector>
#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>

namespace adcontrols {

    class ScanLaw;
    class ADCONTROLSSHARED_EXPORT MSCalibration;

    class MSCalibration {
    public:
        enum ALGORITHM { TIMESQUARED, MULTITURN_NORMALIZED };

        MSCalibration();
        MSCalibration( const boost::uuids::uuid& massSpectrometerClsid );
        MSCalibration( const MSCalibration& );

        const std::string& date() const;

        void setMassSpectrometerClsid( const boost::uuids::uuid& );
        const boost::uuids::uuid& massSpectrometerClsid() const;
        const boost::uuids::uuid& calibrationUuid() const;
        int32_t mode() const;
        void setMode( int32_t );

        const std::vector< double >& coeffs() const;
        void setCoeffs( const std::vector<double>& );

        double compute_mass( double time ) const;
        double compute_time( double mass ) const;

        std::string formulaText( bool ritchText = true ) const;

    private:
        static double compute( const std::vector<double>&, double time );

    private:
        std::string process_date_;
        int32_t mode_;
        std::vector< double > coeffs_;
        boost::uuids::uuid calibrationUuid_;
        boost::uuids::uuid massSpectrometerClsid_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version <= 2 ) {
                // don't delete this --- old data (up to 4.1.7-25) contains this structure with empty values
                int32_t t0_method, algo, time_prefix;
                std::vector< double > t0_coeffs;
                std::wstring calibId;
                ar & BOOST_SERIALIZATION_NVP( process_date_ );
                ar & BOOST_SERIALIZATION_NVP( calibId );
                ar & BOOST_SERIALIZATION_NVP( coeffs_ );
                ar & BOOST_SERIALIZATION_NVP( t0_method );
                ar & BOOST_SERIALIZATION_NVP( t0_coeffs );
                ar & BOOST_SERIALIZATION_NVP( time_prefix );
                ar & BOOST_SERIALIZATION_NVP( algo );
            } else { // v3
                ar & BOOST_SERIALIZATION_NVP( process_date_ );
                ar & BOOST_SERIALIZATION_NVP( mode_ );
                ar & BOOST_SERIALIZATION_NVP( coeffs_ );
                ar & BOOST_SERIALIZATION_NVP( calibrationUuid_ );
                ar & BOOST_SERIALIZATION_NVP( massSpectrometerClsid_ );
            }
        }
    };

}

BOOST_CLASS_VERSION( adcontrols::MSCalibration, 3 )
