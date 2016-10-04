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

#ifndef MOL_HPP
#define MOL_HPP

#include "adchem_global.hpp"
#include <memory>
#include <string>

namespace RDKit { class ROMol; }

namespace adchem {

    class ADCHEMSHARED_EXPORT mol {
    public:
        enum inputType { SMILES, INCHI };
        ~mol();
        mol();
        mol( const mol& );
        mol( const std::string&, inputType t = SMILES );
        inline operator const RDKit::ROMol * () const { return mol_.get(); }
        inline operator RDKit::ROMol * () { return mol_.get(); }
        static std::string smiles( const RDKit::ROMol& );
        static std::string formula( const RDKit::ROMol&, bool separateIsotopes = true, bool abbreviateHIsotopes = false );

        std::string formula() const;
        std::string smiles() const;
        std::string InChI() const;
        static std::string InChIToInChIKey( const std::string& );

    private:
        std::unique_ptr< RDKit::ROMol > mol_;
    };

}

#endif // MOL_HPP
