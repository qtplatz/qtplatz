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

#ifndef SDFILE_HPP
#define SDFILE_HPP

#include "adchem_global.hpp"
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace RDKit {
    class SDMolSupplier;
    class ROMol;
}

namespace adchem {

    class sdfile_iterator;

    class ADCHEMSHARED_EXPORT SDFileData;

    class SDFileData {
        std::vector< std::pair< std::string, std::string > > dataItems_;
        std::string svg_;
        std::string smiles_;
        std::string formula_;
        size_t index_;
    public:
        SDFileData();
        SDFileData( const SDFileData& );
        SDFileData( const sdfile_iterator& );
        const std::string& svg() const { return svg_; }
        const std::string& smiles() const { return smiles_; }
        const std::string& formula() const { return formula_; }
        const std::vector< std::pair< std::string, std::string > > dataItems() const { return dataItems_; }
        size_t index() const { return index_; }
    };

    class ADCHEMSHARED_EXPORT SDFile {
    public:
        typedef sdfile_iterator iterator;
        typedef const sdfile_iterator const_iterator;
        typedef size_t size_type;
        typedef RDKit::ROMol value_type;

        SDFile( const std::string& filename, bool sanitize = true, bool removeHs = true, bool strictParsing = true );
        operator bool() const { return molSupplier_ != 0; }

        std::shared_ptr< RDKit::SDMolSupplier >& molSupplier() { return molSupplier_; }

        size_type size() const;
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;
        std::string itemText( const sdfile_iterator& );
        static bool parseItemText( const std::string&, std::map< std::string, std::string >& );
        static std::vector< std::pair< std::string, std::string > > parseItemText( const std::string& );

        std::vector< SDFileData > toData( std::function< bool(size_t) > progress = [](size_t){ return false; } );

    private:
        std::shared_ptr< RDKit::SDMolSupplier > molSupplier_;
        std::string filename_;
    };

    class ADCHEMSHARED_EXPORT sdfile_iterator {
    public:
        using value_type        = RDKit::ROMol;
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type*;  // or also value_type*
        using reference         = RDKit::ROMol&;
    private:
        RDKit::SDMolSupplier& supplier_;
        uint32_t idx_;

    public:
        sdfile_iterator( RDKit::SDMolSupplier& supplier, size_t idx );
        sdfile_iterator( const sdfile_iterator& );

        reference operator*() const;
        pointer operator->();

        // Prefix increment
        sdfile_iterator& operator++() { idx_++; return *this; }

        // Postfix increment
        sdfile_iterator operator++(int) { sdfile_iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator== (const sdfile_iterator& a, const sdfile_iterator& b) { return a.idx_ == b.idx_; };
        friend bool operator!= (const sdfile_iterator& a, const sdfile_iterator& b) { return a.idx_ != b.idx_; };

        std::string itemText() const;
        uint32_t index() const { return idx_; }
    };
}

#endif // SDFILE_HPP
