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

#include "candidate.hpp"
#include "isopeak.hpp"

namespace lipidid {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const candidate& t )
    {
        jv = {{ "exact_mass", t.exact_mass() }
            , { "formula",    t.formula() }
            , { "adduct",     t.adduct() }
            , { "mass_error", t.mass_error() }
            , { "isotope",    t.isotope() }
            , { "InChIKey",   t.inchiKeys() } // document::instance()->inchKeys( candidate.formula ) }};
        };
    }

}

using namespace lipidid;

candidate::~candidate()
{
}

candidate::candidate( double m
                      , const std::string& f                 // formula
                      , const std::string& a                 // adduct/lose
                      , double e                             // mass_error
                      , std::vector< lipidid::isoPeak >&& i    // isotope match
                      , std::vector< std::string >&& keys )   // list of inchikey
    : exact_mass_( m )
    , formula_( f )
    , adduct_( a )
    , mass_error_( e )
    , isotope_( i )
    , inchiKeys_( keys )
{
}

candidate::candidate( const candidate& t ) : exact_mass_( t.exact_mass_ )
                                           , formula_( t.formula_ )
                                           , adduct_( t.adduct_ )
                                           , mass_error_( t.mass_error_ )
                                           , isotope_( t.isotope_ )
                                           , inchiKeys_( t.inchiKeys_ )
{
}
