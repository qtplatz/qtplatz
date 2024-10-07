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

#include "jstrest.hpp"
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>
#include <sstream>
#include <string>
#include <regex>

namespace adcontrols {

    void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const JSTREST& );
    JSTREST tag_invoke( const boost::json::value_to_tag< JSTREST >&, const boost::json::value& jv );

    JSTREST::~JSTREST()
    {
    }

    JSTREST::JSTREST() : port_( "https" )
                       , host_( "api.jstage.jst.go.jp" )
                       , target_( "/searchapi/do?service=3&material=Mass%20Spectrometry" )
    {
    }

    JSTREST::JSTREST( const JSTREST& t ) : port_( t.port_ )
                                         , host_( t.host_ )
                                         , target_( t.target_ )
    {
    }

    void
    JSTREST::set_url( const std::string& url )
    {
        std::tie( port_, host_, target_ ) = parse_url( url );
    }

    std::string
    JSTREST::host() const
    {
        return host_;
    }

    void
    JSTREST::set_host( const std::string& t )
    {
        host_ = t;
    }

    std::string
    JSTREST::port() const
    {
        return port_;
    }

    void
    JSTREST::set_port( const std::string& t )
    {
        port_ = t;
    }

    std::string
    JSTREST::target() const
    {
        return target_;
    }

    void
    JSTREST::set_target( const std::string& t )
    {
        target_ = t;
    }


    std::string
    JSTREST::to_url( const JSTREST& t )
    {
        std::ostringstream o;
        o << t.port() << "://" << t.host() << t.target();
        return o.str();
    }

    std::tuple< std::string, std::string, std::string > // port, host, taarget
    JSTREST::parse_url( const std::string& url )
    {
        std::regex re( R"((.*)://([^/]*)(/.*)$)", std::regex::extended );
        std::match_results< typename std::basic_string< char >::const_iterator > match;
        if ( std::regex_match( url, match, re ) ) {
            return std::make_tuple( match[1].str(), match[2].str(), match[3].str() );
        } else {
            std::regex re( R"((.*)://([^/]*)$)", std::regex::extended );
            if ( std::regex_match( url, match, re ) ) {
                return std::make_tuple( match[1].str(), match[2].str(), std::string() );
            } else {
                std::regex re( R"((.*)$)", std::regex::extended );
                if ( std::regex_match( url, match, re ) ) {
                    return std::make_tuple( match[1].str(), std::string{}, std::string{} );
                } else {
                    ADDEBUG() << "--- not match -- ";
                }
            }
        }
        return {};
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const JSTREST& t )
    {
        jv = {
            { "port", t.port_ }
            , { "host", t.host_ }
            , { "target", t.target_ }
        };
    }

    JSTREST
    tag_invoke( const boost::json::value_to_tag< JSTREST >&, const boost::json::value& jv )
    {
        using namespace adportable::json;

        if ( jv.kind() == boost::json::kind::object ) {
            JSTREST t;
            auto obj = jv.as_object();
            extract( obj, t.port_, "port" );
            extract( obj, t.host_, "host" );
            extract( obj, t.target_, "target" );
            return t;
        }
        return {};
    }

}
