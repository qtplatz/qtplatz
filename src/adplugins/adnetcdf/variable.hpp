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

#pragma once

#include <string>
#include <netcdf.h>
#include <ostream>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>

namespace adnetcdf {
    namespace netcdf {

        class variable {
        public:
            typedef std::tuple< int, std::string, nc_type, int, int > value_type;
            enum { _varid, _name, _type, _ndims, _natts } value_id;

            variable();
            variable( const variable& );

            variable( int varid, const std::string& name, nc_type type, int ndims, std::vector<int>&& dimids, int natts );
            variable( const value_type & );
            value_type value() const;
            const std::vector< int > dimids() const;
            inline int varid() const { return varid_; };
            inline const std::string& name() const { return name_; };
        private:
            int varid_;
            std::string name_;
            nc_type type_;
            int ndims_;
            std::vector< int > dimids_;
            int natts_;
            friend void tag_invoke( boost::json::value_from_tag, boost::json::value&, const variable& );
            // friend variable tag_invoke( boost::json::value_to_tag< variable >&, const boost::json::value& jv );
        };

        void tag_invoke( boost::json::value_from_tag, boost::json::value&, const variable& );
        // variable tag_invoke( boost::json::value_to_tag< variable >&, const boost::json::value& jv );

    } // namespace netcdf
}
