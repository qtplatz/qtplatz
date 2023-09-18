/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adchem_global.hpp"
#include <GraphMol/MolDraw2D/MolDraw2DHelpers.h>
#include <memory>
#include <string>
#include <tuple>
#include <iterator>
#include <optional>

namespace RDKit { class ROMol; }

namespace adchem {

    class ADCHEMSHARED_EXPORT SmilesToSVG;

    class SmilesToSVG {
    public:
        typedef std::tuple< std::string, std::string > value_type; // formula,smiles,svg
        std::optional< value_type > operator()( const std::string& smiles, RDKit::DrawColour&& = { 1, 1, 1, 1 } ) const;
    };

}
