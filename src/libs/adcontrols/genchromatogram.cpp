/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "genchromatogram.hpp"
#include "targeting/candidate.hpp"
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>

namespace adcontrols {

    // this class is corresponding to targeting::Candidate class
    // due to historical reason, different json interface has been implemented.

    struct ADCONTROLSSHARED_EXPORT GenChromatogram;

    GenChromatogram::GenChromatogram() : exact_mass{0}
                                       , exact_abundance{0}
                                       , mass{0}
                                       , time{0}
                                       , index{0}
                                       , proto{0}
                                       , selected{false}
    {
    }

    GenChromatogram::GenChromatogram( const GenChromatogram& t )
        : formula{ t.formula }
        , display_name{ t.display_name }
        , exact_mass{ t.exact_mass }
        , exact_abundance{ t.exact_abundance }
        , mass{ t.mass }
        , time{ t.time }
        , index{ t.index }
        , proto{ t.proto }
        , selected{ t.selected }
        , isotopes{ t.isotopes }
    {
    }

    GenChromatogram::GenChromatogram( const targeting::Candidate& t, bool __selected )
        : formula{ t.formula }
        , display_name{ t.display_name }
        , exact_mass{ t.exact_mass }
        , exact_abundance{ 100 }
        , mass{ t.mass }
        , time{ -1. }
        , index{ int(t.idx) }
        , proto{ t.fcn }
        , selected{ __selected }
        , isotopes{ t.isotopes }
    {
    }



    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const GenChromatogram& t ) {
            jv = boost::json::object{
                {   "formula",         t.formula }
                , { "display_name",    t.display_name }
                , { "exact_mass",      t.exact_mass }
                , { "exact_abundance", t.exact_abundance }
                , { "mass",            t.mass }
                , { "time",            t.time }
                , { "index",           t.index }
                , { "proto",           t.proto }
                , { "selected",        t.selected }
                , { "children",        t.isotopes } // historical reason
            };
    }

    GenChromatogram
    tag_invoke( boost::json::value_to_tag< GenChromatogram >&, const boost::json::value& jv ) {
        GenChromatogram t;
        using namespace adportable::json;
        if ( jv.is_object() ) {
            auto obj = jv.as_object();
            extract( obj, t.formula ,           "formula" );
            extract( obj, t.display_name ,      "display_name" );
            extract( obj, t.exact_mass ,        "exact_mass" );
            extract( obj, t.exact_abundance ,   "exact_abundance" );
            extract( obj, t.mass ,              "mass" );
            extract( obj, t.time ,              "time" );
            extract( obj, t.index ,             "index" );
            extract( obj, t.proto ,             "proto" );
            extract( obj, t.selected ,          "selected" );
            extract( obj, t.isotopes        ,   "children" );
        }
        return t;
    }
}
