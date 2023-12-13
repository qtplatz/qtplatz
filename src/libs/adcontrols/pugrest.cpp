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

#include "pugrest.hpp"
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>
#include <sstream>
#include <string>

namespace adcontrols {

    void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const PUGREST& );
    PUGREST tag_invoke( const boost::json::value_to_tag< PUGREST >&, const boost::json::value& jv );

    PUGREST::~PUGREST()
    {
    }

    PUGREST::PUGREST() : autocomplete_( false )
                       , identifier_( "apap" )
                       , namespace_( "name" )
                       , domain_( "compound" )
                       , property_( { "CanonicalSMILES"
                               , "InChI"
                               , "MolecularFormula"
                               , "ExactMass"
                               , "XLogP"
                               , "Title"
                               //, "InChIKey"
                               //, "MolecularWeight"
                               //, "IUPACName"
        })
    {
    }

    PUGREST::PUGREST( const PUGREST& t ) : autocomplete_( t.autocomplete_ )
                                         , identifier_( t.identifier_ )
                                         , property_( t.property_ )
                                         , namespace_( t.namespace_ )
                                         , domain_( t.domain_ )
                                         , url_( t.url_ )
    {
    }

    bool
    PUGREST::pug_autocomplete() const
    {
        return autocomplete_;
    }

    void
    PUGREST::set_pug_autocomplete( bool f )
    {
        autocomplete_ = f;
    }

    std::string
    PUGREST::pug_identifier() const
    {
        return identifier_;
    }

    void
    PUGREST::set_pug_identifier( const std::string& t )
    {
        identifier_ = t;
    }

    void
    PUGREST::set_pug_property( const std::string& t, bool add )
    {
        if ( add ) {
            if ( std::find( property_.begin(), property_.end(), t ) == property_.end() )
                property_.emplace_back( t );
        } else {
            auto it = std::find( property_.begin(), property_.end(), t );
            if ( it != property_.end() )
                property_.erase( it );
        }
    }

    const std::vector< std::string >&
    PUGREST::pug_properties() const
    {
        return property_;
    }

    void
    PUGREST::set_pug_properties( std::vector< std::string >&& t )
    {
        property_ = std::move( t );
    }

    std::string
    PUGREST::pug_domain() const
    {
        return domain_;
    }

    void
    PUGREST::set_pug_domain( const std::string& t )
    {
        domain_ = t;
    }

    std::string
    PUGREST::pug_namespace() const
    {
        return namespace_;
    }

    void
    PUGREST::set_pug_namespace( const std::string& t )
    {
        namespace_ = t;
    }

    std::string
    PUGREST::pug_url() const
    {
        return url_;
    }

    void
    PUGREST::set_pug_url( const std::string& url )
    {
        url_ = url;
    }

    std::string
    PUGREST::to_url( const PUGREST& t, bool host )
    {
        std::ostringstream o;
        if ( host )
            o << "https://pubchem.ncbi.nlm.nih.gov";
        o << "/rest";
        if ( t.pug_autocomplete() ) {
            o << "/autocomplete"
              << "/" << t.pug_domain()
              << "/" << t.pug_identifier()
              << "/json?limit=20";
        } else {
            o << "/pug"
              << "/" << t.pug_domain()            // compound
              << "/" << t.pug_namespace()         // name
              << "/" << t.pug_identifier()
              << "/property/";
            for ( auto  it = t.pug_properties().begin(); it != t.pug_properties().end(); ++it ) {
                o << *it;
                if ( (it + 1) != t.pug_properties().end() )
                    o << ",";
            }
            o << "/JSON";
        }
        return o.str();
    }

    std::tuple< std::string, std::string, std::string > // port, host, rest
    PUGREST::parse_url( const std::string& url )
    {
        std::tuple< std::string, std::string, std::string > t{ "https", "pubchem.ncbi.nlm.nih.gov", url };
        std::string::size_type bpos{0}, pos{0};
        if ( (pos = url.find( "://", bpos )) != std::string::npos ) {
            std::get<0>(t) = url.substr( 0, pos );
            bpos = pos + 3;
        }
        if ( (pos = url.find( "/", bpos + 1 )) != std::string::npos ) {
            std::get<1>(t) = url.substr( bpos, (pos - bpos));
            std::get<2>(t) = url.substr( pos );
        }
        return t;
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const PUGREST& t )
    {
        jv = {
            { "autocomplete", t.autocomplete_ }
            ,{ "identifier", t.identifier_ }
            ,{ "property", boost::json::value_from( t.property_ ) }
            ,{ "namespace", t.namespace_ }
            ,{ "domain", t.domain_ }
            ,{ "url", t.url_ }
        };
    }

    PUGREST
    tag_invoke( const boost::json::value_to_tag< PUGREST >&, const boost::json::value& jv )
    {
        using namespace adportable::json;

        if ( jv.kind() == boost::json::kind::object ) {
            PUGREST t;
            auto obj = jv.as_object();
            extract( obj, t.autocomplete_, "autocomplete" );
            extract( obj, t.identifier_, "identifier" );
            extract( obj, t.property_, "property" );
            extract( obj, t.namespace_, "namespace" );
            extract( obj, t.domain_, "domain" );
            extract( obj, t.url_, "url" );
            return t;
        }
        return {};
    }

}
