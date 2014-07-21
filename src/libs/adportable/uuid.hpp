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

#ifndef UUID_HPP
#define UUID_HPP

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace adportable {

    class uuid {
        uuid( const uuid& ) = delete;
    public:
        uuid();
        boost::uuids::uuid operator()();
        boost::uuids::uuid operator()( const boost::uuids::uuid& base, const std::string& name );
        template< typename char_type > std::basic_string< char_type > operator()( const boost::uuids::uuid& uuid ) {
            return boost::lexical_cast<std::basic_string<char_type>>(uuid);
        }
        template< typename char_type > boost::uuids::uuid& operator()( const std::basic_string< char_type >& str ) {
            return boost::lexical_cast<boost::uuids::uuid>(str);
        }
    };

}

#endif // UUID_HPP
