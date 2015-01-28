/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <utility>
#include <cstdint>
#include <string>

namespace adportable {

    class mslockref {
    public:
        ~mslockref();
        mslockref();
        mslockref( const mslockref& );
        mslockref( const std::string& formula, double time );

        std::vector< std::pair< std::string, double > >& vec() { return refs_; }        
        const std::vector< std::pair< std::string, double > >& vec() const { return refs_; }
        
    private:
        std::vector< std::pair< std::string, double > > refs_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive& ar, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP (refs_);
        }
    };
}

BOOST_CLASS_VERSION( adportable::mslockref, 1)

