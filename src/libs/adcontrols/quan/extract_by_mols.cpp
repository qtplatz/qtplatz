/**************************************************************************
** Copyright (C) 2021-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2021-2022 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "extract_by_mols.hpp"
#include "targeting_candidate.hpp"
#include <adportable/json/extract.hpp>
#include <adportable/debug.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/json.hpp>

namespace adcontrols {
    namespace quan {

        // extract_by_mols
        void tag_invoke( const boost::json::value_from_tag
                         , boost::json::value& jv, const extract_by_mols& t )
        {
            jv = {{ "molid", boost::uuids::to_string( t.molid ) }
                , { "wform_type", t.wform_type }
                , { "moltable", boost::json::value_from( t.moltable_ ) }
                , { "auto_target_candidate", ( t.auto_target_candidate ?
                                               boost::json::value_from( *t.auto_target_candidate ) : boost::json::value{} )}
                , { "msref",    (t.msref ? *t.msref : boost::json::value{}) }
                , { "centroid", (t.centroid ? boost::json::value_from( *t.centroid ) : boost::json::value{} ) }
            };
        }

        extract_by_mols tag_invoke( const boost::json::value_to_tag< extract_by_mols >&
                                    , const boost::json::value& jv )
        {
            extract_by_mols t;
            using namespace adportable::json;
            if ( jv.is_object() ) {
                auto obj = jv.as_object();
                extract( obj, t.molid, "molid" );
                extract( obj, t.wform_type, "wform_type" );
                extract( obj, t.moltable_,  "moltable" );
                auto atc = obj.at( "auto_target_candidate" );
                if ( atc.is_object() ) {
                    t.auto_target_candidate = boost::json::value_to< adcontrols::quan::targeting_candidate >( atc );
                }
                auto msref = obj.at( "msref" );
                if ( msref.is_null() ) {
                    t.msref = boost::none;
                } else {
                    t.msref = boost::json::value_to< bool >( msref );
                }
                auto centroid = obj.at( "centroid" );
                if ( centroid.is_null() || boost::json::value_to< std::string >( centroid ) == "" ) {
                    t.centroid = boost::none;
                } else {
                    t.centroid = boost::json::value_to< std::string >( centroid );
                }
            }
            return t;
        }

        ////////////////////////////////////////////
        // ///////////// moltable //////////////////
        void tag_invoke( const boost::json::value_from_tag
                         , boost::json::value& jv, const moltable& t )
        {
            jv = {{ "protocol", t.protocol}
                , { "mass", t.mass }
                , { "width", t.width }
                , { "formula", t.formula }
                , { "adduct", t.adduct }
                , { "tof", t.tof }};
        }


        moltable tag_invoke( const boost::json::value_to_tag< moltable >&
                             , const boost::json::value& jv )
        {
            moltable t;
            using namespace adportable::json;
            if ( jv.is_object() ) {
                auto obj = jv.as_object();
                extract( obj, t.protocol, "protocol" );
                extract( obj, t.mass,     "mass" );
                extract( obj, t.width,    "width" );
                extract( obj, t.formula,  "formula" );
                extract( obj, t.tof,      "tof" );
                if ( obj.contains( "adduct" ) )
                    extract( obj, t.adduct,      "adduct" );
            }
            return t;
        }

    }
}
