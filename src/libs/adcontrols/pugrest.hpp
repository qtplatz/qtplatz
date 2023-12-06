// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2024 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#pragma once

#include "adcontrols_global.h"
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <boost/json/value_from.hpp>
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT PUGREST;


    ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const PUGREST& );
    ADCONTROLSSHARED_EXPORT PUGREST tag_invoke( const boost::json::value_to_tag< PUGREST >&, const boost::json::value& jv );

    class ADCONTROLSSHARED_EXPORT PUGREST;

    class PUGREST {
    public:
        ~PUGREST();
        PUGREST();
        PUGREST( const PUGREST& );

        bool pug_autocomplete() const;
        void set_pug_autocomplete( bool );

        const std::vector< std::string >&  pug_properties() const;
        void set_pug_properties( std::vector< std::string >&& );

        std::string pug_domain() const;
        void set_pug_domain( const std::string& );

        std::string pug_namespace() const;
        void set_pug_namespace( const std::string& );

        std::string pug_identifier() const;
        void set_pug_identifier( const std::string& );

        static std::string to_url( const PUGREST&, bool host = false );

    private:
        bool autocomplete_;
        std::string identifier_;
        std::vector< std::string > property_;  // CanonicalSMILES, ...
        std::string namespace_; // name|cid|inchi, ...
        std::string domain_;

        friend ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const PUGREST& );
        friend ADCONTROLSSHARED_EXPORT PUGREST tag_invoke( const boost::json::value_to_tag< PUGREST >&, const boost::json::value& );
    };
}
