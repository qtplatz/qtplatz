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

#include "smilestosvg.hpp"
#if HAVE_RDKit
#include "drawing.hpp"
#include <RDGeneral/Invariant.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
//#include <GraphMol/FileParsers/FileParsers.h>
//#include <GraphMol/FileParsers/MolSupplier.h>
#include <RDGeneral/RDLog.h>
#endif
#include <adportable/debug.hpp>

using namespace adchem;


adportable::optional< adchem::SmilesToSVG::value_type >
SmilesToSVG::operator()( const std::string& smiles ) const
{
#if HAVE_RDKit
    try {
        if ( auto mol = std::unique_ptr< RDKit::ROMol >( RDKit::SmilesToMol( smiles, 0, false ) ) ) {
            mol->updatePropertyCache( false );
            auto svg = adchem::drawing::toSVG( *mol );
            return std::make_tuple( RDKit::Descriptors::calcMolFormula( *mol, true, false ), svg );
        }
    } catch( std::exception& ex ) {
        ADDEBUG() << "Exception:: " << ex.what();
    }
#endif
    return {};
}
