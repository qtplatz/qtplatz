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

#pragma once

#include "adchem_global.hpp"
#include "sdmol.hpp"
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

#if HAVE_RDKit
#include <GraphMol/RDKitBase.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#endif

// namespace RDKit {
//     class SDMolSupplier;
//     class ROMol;
// }

namespace adchem {

    class ADCHEMSHARED_EXPORT SDFile;

    class SDFile : public std::enable_shared_from_this< SDFile > {
        SDFile( const SDFile& ) = delete;
        SDFile& operator = ( const SDFile& ) = delete;
        SDFile( const std::string& filename, bool sanitize = true, bool removeHs = true, bool strictParsing = true );
    public:
        // https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const/8147101#8147101
        template<typename ...Args> std::shared_ptr<SDFile> static create(Args&&...arg) {
            struct enable_make_shared : public SDFile {
                enable_make_shared(Args&&...arg) : SDFile(std::forward<Args>(arg)...) {}
            };
            return std::make_shared<enable_make_shared>(std::forward<Args>(arg)...);
        }

        operator bool() const;
        size_t size() const;
        RDKit::SDMolSupplier& molSupplier();
        inline const std::string& filename() const { return filename_; }

        SDMol at( size_t );
        std::vector< SDMol > populate( std::function<void(size_t)> progrss = [](size_t){} );

        static std::vector< std::pair< std::string, std::string > > parseItemText( const std::string& );

    private:
        std::unique_ptr< RDKit::SDMolSupplier > molSupplier_;
        std::string filename_;
    };
}
