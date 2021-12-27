// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022 MS-Cheminformatics LLC
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

#include "extract.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>

namespace boost { namespace uuids { class uuid; } };

namespace adportable {
    namespace json {

        template<>
        ADPORTABLESHARED_EXPORT
        void extract( const boost::json::object& obj, boost::uuids::uuid& t, boost::json::string_view key )  {
            try {
                t = boost::lexical_cast< boost::uuids::uuid >( boost::json::value_to<std::string>( obj.at( key ) ) );
            } catch ( std::exception& ex ) {
                BOOST_THROW_EXCEPTION(std::runtime_error("adportab;e/json/extract<> exception"));
            }
        }

    } // namespace json
} // namespace adportable
