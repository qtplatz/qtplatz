/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "candidate.hpp"
#include "../constants.hpp"
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>

namespace adcontrols {

    namespace targeting {

        Candidate::Candidate() : idx(0)
                               , fcn(0)
                               , charge(1)
                               , mass(0)
                               , exact_mass(0)
                               , score(0)
                               , selected( false )
        {
        }

        Candidate::Candidate( const Candidate& t ) : idx( t.idx )
                                                   , fcn( t.fcn )
                                                   , charge( t.charge )
                                                   , mass( t.mass )
                                                   , formula( t.formula )
                                                   , exact_mass( t.exact_mass )
                                                   , score( t.score )
                                                   , isotopes( t.isotopes )
                                                   , synonym( t.synonym )
                                                   , display_name( t.display_name )
                                                   , selected( t.selected )
        {
        }


        Candidate::Candidate( uint32_t _idx
                              , int32_t _fcn
                              , int32_t _charge
                              , double _mass
                              , double _exact_mass
                              , const std::string& _formula
                              , const std::string& _synonym
                              , const std::string& name )
            : idx( _idx )
            , fcn( _fcn )
            , charge( _charge )
            , mass( _mass )
            , formula( _formula )
            , exact_mass( _exact_mass )
            , score( 0 )
            , synonym( _synonym )
            , display_name( name )
            , selected( false )
        {
        }

        void
        tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const Candidate& t )
        {
            jv = boost::json::object{ { "Candidate"
                    , {
                        {   "idx",          t.idx         }
                        , { "proto",        t.fcn         }
                        , { "charge",       t.charge      }
                        , { "mass",         t.mass        }
                        , { "formula",      t.formula     }
                        , { "exact_mass",   t.exact_mass  }
                        , { "score",        t.score       }
                        , { "isotopes",     t.isotopes    }
                        , { "synonym",      t.synonym     }
                        , { "display_name", t.display_name}
                        , { "selected",     t.selected    }
                    }
                }};
        }

        Candidate
        tag_invoke( boost::json::value_to_tag< Candidate >&, const boost::json::value& jv )
        {
            Candidate t;
            using namespace adportable::json;
            if ( jv.is_object() ) {
                auto obj = jv.as_object();
                extract( obj, t.idx          , "idx"          );
                extract( obj, t.fcn          , "proto"        );
                extract( obj, t.charge       , "charge"       );
                extract( obj, t.mass         , "mass"         );
                extract( obj, t.formula      , "formula"      );
                extract( obj, t.exact_mass   , "exact_mass"   );
                extract( obj, t.score        , "score"        );
                extract( obj, t.isotopes     , "isotopes"     );
                extract( obj, t.synonym      , "synonym"      );
                extract( obj, t.display_name , "display_name" );
                extract( obj, t.selected     , "selected"     );
            }
            return t;
        }


    }
}
