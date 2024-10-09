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

    class ADCONTROLSSHARED_EXPORT figshareREST;

    class figshareREST {
    public:
        ~figshareREST();
        figshareREST();
        figshareREST( const figshareREST& );

        void set_url( const std::string& );

        std::string host() const;
        void set_host( const std::string& );

        std::string port() const;
        void set_port( const std::string& );

        std::string target() const;
        void set_target( const std::string& );

        static std::string to_url( const figshareREST& );

        static std::tuple< std::string   // port 'https'
                           , std::string // host
                           , std::string // target
                           >  parse_url( const std::string& );

    private:
        std::string port_;
        std::string host_;
        std::string target_;

        friend ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const figshareREST& );
        friend ADCONTROLSSHARED_EXPORT figshareREST tag_invoke( const boost::json::value_to_tag< figshareREST >&, const boost::json::value& );
    };
}
