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
#include <boost/uuid/uuid.hpp>
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

        int32_t idx_;         // index on centroid spectrum
        int32_t fcn_;         // function (protocol) id on centroid spectrum
        int64_t compoundId_;  // uniqId on compound class ( not identified if negative )
        double intensity_;    // area | height from chromatogram/spectrum
        double amounts_;      // result
        double mass_;         // observed mass
        double tR_;           // observed retention time
    private:
        std::string formula_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( compoundId_ )
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
