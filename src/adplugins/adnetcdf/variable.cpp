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

#include "variable.hpp"
#include <iomanip>
#include <adportable/debug.hpp>
#include "nc_type.hpp"

namespace adnetcdf {
    namespace netcdf {

        variable::variable() : varid_(0)
                             , name_{}
                             , type_(0)
                             , ndims_(0)
                             , dimids_{}
                             , natts_(0)
        {
        }

        variable::variable( const variable& t ) : varid_( t.varid_ )
                                                , name_( t.name_ )
                                                , type_( t.type_ )
                                                , ndims_( t.ndims_ )
                                                , dimids_( t.dimids_ )
                                                , natts_( t.natts_ )
        {
        }

        variable::variable( int varid
                            , const std::string& name
                            , nc_type type
                            , int ndims
                            , std::vector< int >&& dimids
                            , int natts )  : varid_( varid )
                                           , name_( name )
                                           , type_( type )
                                           , ndims_( ndims )
                                           , dimids_( std::move( dimids ) )
                                           , natts_( natts )
        {
        }

        variable::value_type
        variable::value() const
        {
            return { varid_, name_, type_, ndims_, natts_ };
        }

        void
        tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const variable& t )
        {
            using namespace adnetcdf::netcdf;
            jv = boost::json::value{{ "var"
                    , {
                        { "varid", t.varid_ }
                        , { "name", t.name_ }
                        , { "typid", t.type_ }
                        , { "type", nc_type_name( t.type_, nc_types_t{} ) }
                        , { "ndims", t.ndims_ }
                        , { "dimids", t.dimids_ }
                        , { "natts", t.natts_ }
                    }
                }};
        }

    } // namespace netcdf
}
