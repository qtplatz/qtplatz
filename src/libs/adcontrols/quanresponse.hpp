/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANRESPONSE_HPP
#define QUANRESPONSE_HPP

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT QuanResponse {
    public:
        ~QuanResponse();
        QuanResponse();
        QuanResponse( const QuanResponse& t );

        const char * formula() const { return formula_.c_str(); }
        void formula( const char * t ) { formula_ = t; }

        const boost::uuids::uuid& uuid_cmpd_table() const { return idTable_; }
        void uuid_cmpd_table( const boost::uuids::uuid& u ) { idTable_ = u; }

        void uuid_cmpd( const boost::uuids::uuid& u ) { idCompound_ = u; }
        const boost::uuids::uuid& uuid_cmpd() const { return idCompound_; }

        int32_t idx_;                    // index on centroid spectrum
        boost::uuids::uuid idTable_;     // foreign key reference to QuanCompounds (a file of molecles)
        boost::uuids::uuid idCompound_;  // foreign key reference to QuanCompound (a molecule)
        boost::uuids::uuid dataGuid_;          // reference to spectrum|chromatogram data on 'adfs' file system
        int32_t fcn_;                    // function (protocol) id on centroid spectrum
        double intensity_;               // area | height from chromatogram/spectrum
        double amounts_;                 // result
        double mass_;                    // observed mass
        double tR_;                      // observed retention time
        uint64_t countTimeCounts_;       // count of time-counts
        uint64_t countTriggers_;         // count of triggers

        int32_t peakIndex() const;
        const boost::uuids::uuid& idTable() const;
        const boost::uuids::uuid& idCompound() const;
        const boost::uuids::uuid& dataGuid() const;
        int32_t fcn() const;
        double intensity() const;
        double amounts() const;
        double mass() const;
        double tR() const;
        uint64_t countTimeCounts() const;
        uint64_t countTriggers() const;

        void setPeakIndex( int32_t );
        boost::uuids::uuid& idTable();
        boost::uuids::uuid& idCompound();
        void setDataGuid( const std::wstring& );
        void setDataGuid( const boost::uuids::uuid& );
        void setFcn( int32_t );
        void setIntensity( double );
        void setAmounts( double );
        void setMass( double );
        void set_tR( double );
        void setCountTimeCounts( uint64_t );
        void setCountTriggers( uint64_t );

    private:
        std::string formula_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP(  dataGuid_ );
                ar & BOOST_SERIALIZATION_NVP(  idCompound_ );
                ar & BOOST_SERIALIZATION_NVP( idTable_ );
                ar & BOOST_SERIALIZATION_NVP( formula_ );
                ar & BOOST_SERIALIZATION_NVP( idx_ );
                ar & BOOST_SERIALIZATION_NVP( fcn_ );
                ar & BOOST_SERIALIZATION_NVP( intensity_ );
                ar & BOOST_SERIALIZATION_NVP( amounts_ );
                ar & BOOST_SERIALIZATION_NVP( mass_ );
                ar & BOOST_SERIALIZATION_NVP( tR_ );
                ar & BOOST_SERIALIZATION_NVP( countTimeCounts_ );
                ar & BOOST_SERIALIZATION_NVP( countTriggers_ );
            } else {
                std::wstring dataGuid;
                if ( version >= 1 )
                    ar & BOOST_SERIALIZATION_NVP( dataGuid );
                ar & BOOST_SERIALIZATION_NVP(  idCompound_ );
                ar & BOOST_SERIALIZATION_NVP( idTable_ );
                ar & BOOST_SERIALIZATION_NVP( formula_ );
                ar & BOOST_SERIALIZATION_NVP( idx_ );
                ar & BOOST_SERIALIZATION_NVP( fcn_ );
                ar & BOOST_SERIALIZATION_NVP( intensity_ );
                ar & BOOST_SERIALIZATION_NVP( amounts_ );
                ar & BOOST_SERIALIZATION_NVP( mass_ );
                ar & BOOST_SERIALIZATION_NVP( tR_ );
                if ( version >= 2 ) {
                    ar & BOOST_SERIALIZATION_NVP( countTimeCounts_ );
                    ar & BOOST_SERIALIZATION_NVP( countTriggers_ );
                }
                if ( ! Archive::is_saving::value )
                    setDataGuid( dataGuid );
            }
        }
    };

}

BOOST_CLASS_VERSION( adcontrols::QuanResponse, 3 )

#endif // QUANRESPONSE_HPP
