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

#pragma once

#include "adportable_global.h"
#include <boost/optional/optional_fwd.hpp>
#include <boost/system/error_code.hpp>
#include <boost/json/value.hpp>
#include <boost/json/value_to.hpp>

namespace boost { namespace uuids { class uuid; } };

namespace adportable {

    class json_helper {
    public:
        static boost::json::value parse( const std::string& );
        static boost::json::value parse( const boost::optional< std::string >& );
        static boost::json::value find( const boost::json::value&, const std::string& keys ); // dot delimited key-list
        static boost::json::value find( const std::string& json, const std::string& keys ); // parse & find
        static boost::json::value find( const boost::optional< std::string >&, const std::string& keys ); // parse & find

        template< typename T > static boost::optional< T > value_to( const boost::json::value& jv, const std::string& keys ) {
            auto t = find( jv, keys );
            if ( !t.is_null() )
                return boost::json::value_to< T >( t );
            return {};
        }
    };

    // workaround for boost::uuids::uuid since boost::json::value_to< uuid > does not work on boost_1.75
    template<> boost::optional< boost::uuids::uuid > json_helper::value_to( const boost::json::value& jv, const std::string& keys );

    ADPORTABLESHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const boost::uuids::uuid& );

// ADPORTABLESHARED_EXPORT
// boost::uuids::uuid tag_invoke( boost::json::value_to_tag< boost::uuids::uuid>&, const boost::json::value& jv );
}
