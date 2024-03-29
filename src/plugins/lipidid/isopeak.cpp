/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "isopeak.hpp"
#include <adportable/json/extract.hpp>

namespace lipidid {

    isoPeak::isoPeak( const std::pair< double, double >& t
                      , bool found
                      , size_t peak_index
                      , double mass_error
                      , double ra_error ) : computed_isotope_( t )
                                          , matched_isotope_( { found, peak_index, mass_error, ra_error } )
    {
    }

    isoPeak::isoPeak( const isoPeak& t ) : computed_isotope_( t.computed_isotope_ )
                                         , matched_isotope_( t.matched_isotope_ )
    {
    }

    isoPeak&
    isoPeak::operator = ( std::tuple< bool, size_t, double, double >&& t)
    {
        matched_isotope_ = std::move( t );
        return *this;
    }

    void
    tag_invoke( const boost::json::value_from_tag, boost::json::value& jv, const isoPeak& t )
    {
        jv = boost::json::object{
            {"computed_isotope", boost::json::value_from( t.computed_isotope_ ) }
            , { "matched_isotope", boost::json::value_from( t.matched_isotope_ ) }
        };
    }

    isoPeak
    tag_invoke( const boost::json::value_to_tag< isoPeak >&, const boost::json::value& jv )
    {
        isoPeak t{{0,0}};
        if ( jv.is_object() ) {
            auto obj = jv.as_object();
            adportable::json::extract( obj, t.computed_isotope_, "computed_isotope" );
            adportable::json::extract( obj, t.matched_isotope_,  "matched_isotope" );
        }
        return t;
    }
}
