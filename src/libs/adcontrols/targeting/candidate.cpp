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

namespace adcontrols {

    namespace targeting {

        Candidate::Candidate() : idx(0)
                               , fcn(0)
                               , charge(1)
                               , mass(0)
                               , exact_mass(0)
                               , score(0)
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
                                                   , key( t.key )
        {
        }


        Candidate::Candidate( uint32_t _idx
                              , uint32_t _fcn
                              , int32_t _charge
                              , double _mass
                              , double _exact_mass
                              , const std::string& _formula
                              , const std::string& _synonym
                              , const std::string& _key )
            : idx( _idx )
            , fcn( _fcn )
            , charge( _charge )
            , mass( _mass )
            , formula( _formula )
            , exact_mass( _exact_mass )
            , score( 0 )
            , synonym( _synonym )
            , key( _key )
        {
        }

    }
}
