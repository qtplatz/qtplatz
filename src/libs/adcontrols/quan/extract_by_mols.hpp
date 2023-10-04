/**************************************************************************
** Copyright (C) 2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "../adcontrols_global.h"
#include "targeting_candidate.hpp"
#include <boost/json/fwd.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/optional.hpp>
#include <string>

namespace boost {  namespace json { class object; } }

namespace adcontrols {
    namespace quan {

        struct targeting_candidate;

        struct ADCONTROLSSHARED_EXPORT extract_by_mols;

        // classes defined here are partially using due to histrial reason
        // see adprocessor/mschromatogramextroactor_v3.cpp

        struct moltable {
            int protocol;
            double mass;
            double width;
            std::string formula;
            std::string adduct;
            double tof;
            moltable() : protocol(0), mass(0), width(0), tof(0) {
            }

            moltable( int tprotocol, double tmass, double twidth, const std::string& tformula, double ttof, const std::string& tadduct )
                : protocol( tprotocol )
                , mass( tmass )
                , width( twidth )
                , formula( tformula )
                , tof( ttof )
                , adduct( tadduct ) {
            }

            moltable( const moltable& t ) : protocol( t.protocol )
                                          , mass( t.mass )
                                          , width( t.width )
                                          , formula( t.formula )
                                          , tof( t.tof )
                                          , adduct( t.adduct ) {
            }
        };

        struct extract_by_mols {
            boost::uuids::uuid molid; // "molid": "4f62526d-e48d-40e2-8e65-5904e444b2c1",
            std::string wform_type;   // "centroid",
            moltable moltable_;
            boost::optional< targeting_candidate > auto_target_candidate; // can be null
            boost::optional< bool > msref;
            boost::optional< std::string > centroid;
            extract_by_mols() {}
            extract_by_mols( const extract_by_mols& t) : molid( t.molid )
                                                       , wform_type( t.wform_type )
                                                       , moltable_( t.moltable_ )
                                                       , auto_target_candidate( t.auto_target_candidate )
                                                       , msref( t.msref )
                                                       , centroid( t.centroid ) {
            }
        };

        // extract_by_mols
        ADCONTROLSSHARED_EXPORT
        void tag_invoke( boost::json::value_from_tag
                         , boost::json::value&, const extract_by_mols& );

        ADCONTROLSSHARED_EXPORT
        extract_by_mols tag_invoke( boost::json::value_to_tag< extract_by_mols >&
                                    , const boost::json::value& jv );

        // moltable
        ADCONTROLSSHARED_EXPORT
        void tag_invoke( boost::json::value_from_tag
                         , boost::json::value&, const moltable& );

        ADCONTROLSSHARED_EXPORT
        moltable tag_invoke( boost::json::value_to_tag< moltable >&
                             , const boost::json::value& jv );

    }
}
