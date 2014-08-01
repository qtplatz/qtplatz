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

#ifndef QUANCOMPOUNDS_HPP
#define QUANCOMPOUNDS_HPP

#include "adcontrols_global.h"
#include "quancompound.hpp"
#include "idaudit.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <string>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_serialize.hpp>

namespace adcontrols {

#if defined _MSC_VER
    template class ADCONTROLSSHARED_EXPORT std::vector < QuanCompound > ;
#endif

    class ADCONTROLSSHARED_EXPORT QuanCompounds  {
    public:
        ~QuanCompounds();
        QuanCompounds();
        QuanCompounds( const QuanCompounds& );
        
        typedef QuanCompound value_type;
        typedef std::vector< value_type > vector_type;
        typedef std::vector< QuanCompound >::iterator iterator_type;
        
        std::vector< QuanCompound >::iterator begin() { return compounds_.begin(); }
        std::vector< QuanCompound >::iterator end() { return compounds_.end(); }
        std::vector< QuanCompound >::const_iterator begin() const { return compounds_.begin(); }
        std::vector< QuanCompound >::const_iterator end() const { return compounds_.end(); }
        void clear() { compounds_.clear(); }
        size_t size() const { return compounds_.size(); }
        QuanCompounds& operator << ( const QuanCompound& t );

        const idAudit& ident() const { return ident_; }
        const boost::uuids::uuid& uuid() const;

    private:
        idAudit ident_;
        std::vector< QuanCompound > compounds_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( ident_ );
            ar & BOOST_SERIALIZATION_NVP( compounds_ );
        }
    };

}

BOOST_CLASS_VERSION( adcontrols::QuanCompounds, 1 )


#endif // QUANCOMPOUNDS_HPP
