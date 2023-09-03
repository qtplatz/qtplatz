// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include "dimension.hpp"
#include <adportable/debug.hpp>
#include <netcdf.h>

namespace adnetcdf {
    namespace netcdf {

//////////////////////////////////////////////////

        dimension::dimension() : dimid_( 0 )
                               , len_( 0 ) {
        }

        dimension::dimension( const dimension& t ) : dimid_( t.dimid_ )
                                                   , name_( t.name_ )
                                                   , len_( t.len_ ) {
        }

        dimension::dimension( int dimid, const std::string& name, size_t& len ) : dimid_( dimid )
                                                                                , name_( name )
                                                                                , len_( len )
        {
        }

        dimension::dimension( const std::tuple< int, std::string, size_t >& t )
        {
            std::tie( dimid_, name_, len_ ) = t;
        }

        std::tuple< int, std::string, size_t >
        dimension::value() const
        {
            return std::make_tuple( dimid_, name_, len_ );
        }

    }
}
