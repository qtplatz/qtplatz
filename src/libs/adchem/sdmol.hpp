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

#pragma once

#if HAVE_RDKit
#include "adchem_global.hpp"
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <GraphMol/ROMol.h>

namespace adchem {

    class SDFile;

    class ADCHEMSHARED_EXPORT SDMol {
        size_t index_;
        std::vector< std::pair< std::string, std::string > > dataItems_;
        std::string ctable_;
        mutable std::unique_ptr< RDKit::ROMol > mol_;
        mutable std::string svg_;
        mutable std::string smiles_;
        mutable std::string formula_;
        mutable double mass_;
    public:
        SDMol();
        SDMol( const SDMol& );
        SDMol& operator = ( const SDMol& );
        SDMol( SDFile *, size_t idx );
        operator bool () const;
        const std::string& ctable() const;

        const std::string& svg() const;
        const std::string& smiles() const;
        const std::string& formula() const;
        double mass() const;
        std::pair< double, double > logP() const;
        const std::vector< std::pair< std::string, std::string > > dataItems() const;
        size_t index() const { return index_; }
        RDKit::ROMol& mol();
        const RDKit::ROMol& mol() const;
    };

}

#endif // HAVE_RDKit
