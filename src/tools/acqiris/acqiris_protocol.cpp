/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "acqiris_protocol.hpp"
#include <boost/uuid/uuid_generators.hpp>

namespace aqdrv4 {

    boost::uuids::uuid clsid_connection_request =
        boost::uuids::string_generator()( "{8fcb150a-74e7-11e6-8f1d-bfbec26f05f1}" );

    boost::uuids::uuid clsid_acknowledge =
        boost::uuids::string_generator()( "{e6eaa666-74f1-11e6-815d-b31b513d9069}" );

    preamble::preamble( const boost::uuids::uuid& uuid ) : clsid( uuid )
                                                         , aug( 0x20160907 )
                                                         , length( 0 )
                                                         , request( 0 )
    {
    }

}
