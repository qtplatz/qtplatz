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
#include <memory>
#include <string>
#include <tuple>
#include <iterator>

namespace RDKit { class SDMolSupplier; class ROMol; }

namespace adchem {

    class ADCHEMSHARED_EXPORT SDMolSupplier;

    class SDMolSupplier {
    public:
        typedef std::tuple< std::string, std::string, std::string > value_type; // formula,smiles,svg

        ~SDMolSupplier();
        SDMolSupplier();
        SDMolSupplier( const std::string& filename );
        void setData( std::string&& pasted );
        size_t size() const;

        value_type operator [] ( uint32_t idx ) const;

#if HAVE_RDKit
        inline RDKit::SDMolSupplier& supplier() { return *supplier_; }
        
        class iterator : public std::iterator<  std::input_iterator_tag     // iterator_category
                                                , RDKit::ROMol              // value_type
                                                , size_t                    // difference_type
                                                , const RDKit::ROMol*       // pointer
                                                , const RDKit::ROMol& > {   // reference
        public:
            explicit iterator( RDKit::SDMolSupplier&, size_t idx = 0 );
            iterator( const iterator& );
            iterator& operator++();
            iterator operator++(int);
            bool operator==(iterator other);
            bool operator!=(iterator other);
            reference operator*() const;
        private:
            RDKit::SDMolSupplier& supplier_;
            size_t idx_;
            std::unique_ptr< RDKit::ROMol > mol_;
        };
    
        iterator begin();
        iterator end();
    private:
        std::unique_ptr< RDKit::SDMolSupplier > supplier_;
#endif
    };

}

