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

#include "adchem_global.hpp"
#include <string>
#include <map>
#include <memory>

namespace RDKit {
    class SDMolSupplier;
    class ROMol;
}

namespace adchem {

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4251)
#endif

    class sdfile_iterator;
    
    class ADCHEMSHARED_EXPORT SDFile {
    public:
        typedef sdfile_iterator iterator;
        typedef const sdfile_iterator const_iterator;
        typedef size_t size_type;
        typedef RDKit::ROMol value_type;

        SDFile( const std::string& filename, bool sanitize = false, bool removeHs = false, bool strictParsing = false );
        operator bool() const { return molSupplier_ != 0; }
        
        std::shared_ptr< RDKit::SDMolSupplier >& molSupplier() { return molSupplier_; }

        size_type size() const;
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;
        //value_type operator [] ( size_type idx ) const;

        static bool parseItemText( const std::string&, std::map< std::string, std::string >& );

    private:
        std::shared_ptr< RDKit::SDMolSupplier > molSupplier_;
        std::string filename_;
    };

    class ADCHEMSHARED_EXPORT sdfile_iterator {
        RDKit::SDMolSupplier& supplier_;
        std::unique_ptr< RDKit::ROMol > mol_;
        size_t idx_;
        bool fetch();
    public:
        sdfile_iterator( RDKit::SDMolSupplier& supplier, size_t idx );
        sdfile_iterator( const sdfile_iterator& );
        bool operator != ( const sdfile_iterator& ) const;
        operator SDFile::value_type * () const;
        const sdfile_iterator& operator ++ ();
        sdfile_iterator operator + ( int ) const;
        std::string itemText() const;
    };

#ifdef _MSC_VER
# pragma warning(pop)
#endif
}

#endif // SDFILE_HPP
