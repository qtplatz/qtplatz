/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#ifndef SDFILE_HPP
#define SDFILE_HPP

#include <memory>
#include <string>
#include <map>

namespace RDKit {
    class SDMolSupplier;
}

namespace chemistry {

    class SDFile {
    public:
        SDFile();
        SDFile( const std::string& filename, bool sanitize = true , bool removeHs = true, bool strictParsing = true );
        operator bool() const { return molSupplier_ != 0; }

        RDKit::SDMolSupplier& molSupplier() { return *molSupplier_; }
        const RDKit::SDMolSupplier& molSupplier() const { return *molSupplier_; }

        static bool associatedData( const std::string&, std::map< std::string, std::string >& );

    private:
        std::shared_ptr< RDKit::SDMolSupplier > molSupplier_;
        std::string filename_;
    };

}

#endif // SDFILE_HPP
