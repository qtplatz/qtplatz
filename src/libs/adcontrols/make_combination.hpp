/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "adcontrols_global.h"
#include "constants_fwd.hpp"
#include <string>
#include <utility>
#include <vector>

namespace adcontrols {

    class IonReactionMethod;
    namespace mol { class molecule; }

    namespace lipidid {

        typedef std::tuple< adcontrols::mol::molecule, adcontrols::mol::molecule, std::string, size_t > molecule_pair_t; // add, sub, key, repeat

        ADCONTROLSSHARED_EXPORT
        std::vector< std::vector< molecule_pair_t > >
        make_combination( const adcontrols::IonReactionMethod& t, adcontrols::ion_polarity );

        ADCONTROLSSHARED_EXPORT
        std::pair< int, std::string >
        to_string( const std::vector< molecule_pair_t >& );

        ADCONTROLSSHARED_EXPORT
        std::pair< adcontrols::mol::molecule, adcontrols::mol::molecule >
        merge_molecule( const std::vector< molecule_pair_t >& alist );
    }

}
