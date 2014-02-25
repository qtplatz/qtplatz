/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MOLECULE_HPP
#define MOLECULE_HPP

#include "adchem_global.hpp"
#include <memory>
#include <string>

namespace RDKit { class ROMol; }

namespace adchem {

    class ADCHEMSHARED_EXPORT molecule {
    public:
        ~molecule();
        molecule();
        molecule( const molecule& );
        molecule( RDKit::ROMol * );

        inline operator bool() const { return mol_ != 0; }
        inline RDKit::ROMol * get() { return mol_; }
        inline const RDKit::ROMol * get() const { return mol_; }
        static RDKit::ROMol * SmilesToMol( const std::string& );
        static std::string MolToSmiles( const RDKit::ROMol& );
        static std::string MolToFormula( RDKit::ROMol&, bool separateIsotopes = true, bool abbreviateHIsotopes = false );

    private:
        RDKit::ROMol * mol_;
    };

}

#endif // MOLECULE_HPP
