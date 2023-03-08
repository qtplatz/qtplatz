/**************************************************************************
** Copyright (C) 2022-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <string>
#include <vector>
#include <adcontrols/make_combination.hpp> // typedef molecule_pair_t

namespace adcontrols { namespace mol { class molecule; } }

namespace lipidid {

    class IonReaction {
        std::string M_;
    public:
        IonReaction();
        IonReaction( const std::string& formula );
        double mass( const std::vector< adcontrols::lipidid::molecule_pair_t >& alist ) const;
        adcontrols::mol::molecule make_molecule( const std::vector< adcontrols::lipidid::molecule_pair_t >& alist ) const;
    };
}
