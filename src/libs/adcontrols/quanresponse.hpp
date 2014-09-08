/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <workaround/boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <compiler/disable_dll_interface.h>

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
        boost::uuids::uuid idTable_; // foreign key reference to QuanCompounds (a file of molecles)
        boost::uuids::uuid idCompound_;  // foreign key reference to QuanCompound (a molecule)
        int32_t fcn_;                    // function (protocol) id on centroid spectrum
        double intensity_;               // area | height from chromatogram/spectrum
        double amounts_;                 // result
        double mass_;                    // observed mass
        double tR_;                      // observed retention time
    private:
        std::string formula_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP(  idCompound_ )
                & BOOST_SERIALIZATION_NVP( idTable_ )                
                & BOOST_SERIALIZATION_NVP( formula_ )                
                & BOOST_SERIALIZATION_NVP( idx_ )                
                & BOOST_SERIALIZATION_NVP( fcn_ )                                
                & BOOST_SERIALIZATION_NVP( intensity_ )
                & BOOST_SERIALIZATION_NVP( amounts_ )
                & BOOST_SERIALIZATION_NVP( mass_ )
                & BOOST_SERIALIZATION_NVP( tR_ )
                ;
        }            
    };

}

#endif // QUANRESPONSE_HPP
