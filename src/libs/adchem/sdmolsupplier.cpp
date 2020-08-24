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

#include "sdmolsupplier.hpp"
#if HAVE_RDKit
#include "drawing.hpp"
#include <RDGeneral/Invariant.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <RDGeneral/RDLog.h>
#endif

using namespace adchem;

SDMolSupplier::~SDMolSupplier()
{
}

SDMolSupplier::SDMolSupplier()
#if HAVE_RDKit    
    : supplier_( std::make_unique< RDKit::SDMolSupplier >() )
#endif
{
}

SDMolSupplier::SDMolSupplier( const std::string& filename )
#if HAVE_RDKit        
    : supplier_( std::make_unique< RDKit::SDMolSupplier >( filename, false, false, false ) )
#endif
{
    // this maybe implemented by using sdfile_parser w/o RDKit.  See sdfile.cpp
}


SDMolSupplier::value_type
SDMolSupplier::operator []( uint32_t idx ) const
{
#if HAVE_RDKit
    auto mol = std::unique_ptr< RDKit::ROMol >( (*supplier_)[ idx ] );
    mol->updatePropertyCache( false );
    auto formula = RDKit::Descriptors::calcMolFormula( *mol, true, false );
    auto smiles = RDKit::MolToSmiles( *mol );
    auto svg = adchem::drawing::toSVG( *mol ); // RDKit::Drawing::DrawingToSVG( drawing );
    return std::make_tuple( formula, smiles, svg );
#else
    return std::make_tuple( "", "", "" );
#endif
}

void
SDMolSupplier::setData( std::string&& pasted )
{
#if HAVE_RDKit    
    supplier_->setData( pasted );
#endif
}

size_t
SDMolSupplier::size() const
{
#if HAVE_RDKit        
    return supplier_->length();
#else
    return 0;
#endif
}

#if HAVE_RDKit
SDMolSupplier::iterator
SDMolSupplier::begin()
{
    return iterator( *supplier_, 0 );
}

SDMolSupplier::iterator
SDMolSupplier::end()
{
    return iterator( *supplier_, size() );
}

SDMolSupplier::iterator::iterator( RDKit::SDMolSupplier& supplier, size_t idx )
    : supplier_( supplier )
    , idx_( idx )
    , mol_( std::unique_ptr< RDKit::ROMol >( supplier_[ idx_ ] ) )
{
}
            
SDMolSupplier::iterator::iterator( const iterator& t )
    : supplier_( t.supplier_ )
    , idx_( t.idx_ )
    , mol_( std::unique_ptr< RDKit::ROMol >( supplier_[ idx_ ] ) )
{
}

SDMolSupplier::iterator&
SDMolSupplier::iterator::operator++()
{
    if ( idx_ + 1 < supplier_.length() )
        ++idx_;
    return *this;
}

SDMolSupplier::iterator
SDMolSupplier::iterator::operator++(int)
{
    auto retval = *this;
    if ( idx_ + 1 < supplier_.length() )
        ++idx_;
    return retval;
}

bool
SDMolSupplier::iterator::operator==(iterator other)
{
    return idx_ == other.idx_;
}

bool
SDMolSupplier::iterator::operator!=(iterator other)
{
    return !(*this == other);
}

SDMolSupplier::iterator::reference
SDMolSupplier::iterator::operator*() const
{
    return *mol_;
}


#endif
