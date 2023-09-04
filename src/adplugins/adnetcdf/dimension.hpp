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
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>

namespace adnetcdf {
    namespace netcdf {

        class dimension {
        public:
            enum { dimid, name, len };
            typedef std::tuple< int, std::string, size_t > value_type;
            dimension();
            dimension( const dimension& );
            dimension( int, const std::string&, size_t& );
            dimension( const value_type& );
            value_type value() const;
        private:
            int dimid_;
            std::string name_;
            size_t len_;
            friend void tag_invoke( boost::json::value_from_tag, boost::json::value&, const dimension& );
        };

    } // namespace netcdf
}
