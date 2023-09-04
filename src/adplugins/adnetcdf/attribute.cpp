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

#include "attribute.hpp"
#include "nc_type.hpp"

namespace adnetcdf {
    namespace netcdf {

        attribute::attribute() : varid_(0)
                               , attid_(0)
                               , name_{}
                               , type_(0)
                               , len_(0)
        {
        }

        attribute::attribute( const attribute& t ) : varid_( t.varid_ )
                                                   , attid_( t.attid_ )
                                                   , name_( t.name_ )
                                                   , type_( t.type_ )
                                                   , len_( t.len_ )
        {
        }

        attribute::attribute( int varid
                              , int attid
                              , const std::string& name
                              , nc_type type
                              , size_t len ) : varid_( varid )
                                              , attid_( attid )
                                              , name_( name )
                                              , type_( type )
                                              , len_( len )
        {
        }

        attribute::value_type
        attribute::value() const
        {
            return { varid_, attid_, name_, type_, len_ };
        }

        void
        tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const attribute& t )
        {
            // using namespace adnetcdf::netcdf;
            jv = boost::json::value{{ "att"
                    , {
                        { "varid",    t.varid_ }
                        , { "attid",   t.attid_ }
                        , { "name",    t.name_ }
                        , { "typeid",  t.type_ }
                        , { "type",    nc_type_name( t.type_, nc_types_t{} ) }
                        , { "type",    t.type_ }
                        , { "len",     t.len_ }
                    }
                }};
        }

    } // namespace netcdf
}
