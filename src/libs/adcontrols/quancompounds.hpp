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
#include <workaround/boost/uuid/uuid.hpp>
#include <workaround/boost/uuid/uuid_io.hpp>
#include <workaround/boost/uuid/uuid_serialize.hpp>
#include <string>
#include <vector>

namespace adcontrols {

#if defined _MSC_VER
    template class ADCONTROLSSHARED_EXPORT std::vector < QuanCompound > ;
#endif

    class ADCONTROLSSHARED_EXPORT QuanCompounds  {
    public:
        ~QuanCompounds();
        QuanCompounds();
        QuanCompounds( const QuanCompounds& );
        QuanCompounds& operator = ( const QuanCompounds& );
        
        typedef QuanCompound value_type;
        typedef std::vector< value_type > vector_type;
        typedef std::vector< QuanCompound >::iterator iterator;
        typedef std::vector< QuanCompound >::const_iterator const_iterator;
        
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        void clear();
        size_t size() const;
        QuanCompounds& operator << ( const QuanCompound& t );

        const idAudit& ident() const;
        const boost::uuids::uuid& uuid() const;

        static bool xml_archive( std::wostream& ostream, const QuanCompounds& );
        static bool xml_restore( std::wistream& istream, QuanCompounds& );

    private:

#   if  defined _MSC_VER
#   pragma warning(disable:4251)
#   endif

        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int );
    };

}

BOOST_CLASS_VERSION( adcontrols::QuanCompounds, 2 )

#endif // QUANCOMPOUNDS_HPP
