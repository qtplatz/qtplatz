/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "wstp.h"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <boost/format.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <cstring>
#include <numeric>
#include <fstream>

#if defined HAVE_RDKit
# if defined _MSC_VER
#  pragma warning( disable: 4267 4018 )
# endif
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

extern "C" {
    double monoIsotopicMass( const char * );
    void standardFormula( const char * );
    void isotopeCluster( const char *, double resolution );
    void formulaFromSMILES( const char * smiles );
}

double
monoIsotopicMass( const char * formula )
{
    double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( adcontrols::ChemicalFormula::split( formula ) );
    return exactMass;
}

void
standardFormula( const char * formula )
{
    auto result = adcontrols::ChemicalFormula::standardFormula( formula );
    WSPutString( stdlink, result.c_str() );
}

void
formulaFromSMILES( const char * smiles )
{
    std::ofstream of( "output.txt" );
    of << "formulaFromSMILES(" << smiles << ")" << std::endl;
    
    if ( auto mol = std::unique_ptr< RDKit::ROMol >( RDKit::SmilesToMol( smiles, 0, false ) ) ) {
        mol->updatePropertyCache( false );
        auto formula = RDKit::Descriptors::calcMolFormula( *mol, true, false );

        of << "formula: " << formula << std::endl;
        WSPutString( stdlink, formula.c_str() );        
    }
}

void
isotopeCluster( const char * formula, double resolving_power )
{
    adcontrols::MassSpectrum ms;

    adcontrols::isotopeCluster()( ms, formula, 1.0, resolving_power );

    long dimensions[ 2 ] = { long(ms.size()), 2 };
    const char * heads[ 2 ] = { "List", "List" };
    std::unique_ptr< double[] > a( std::make_unique< double[] >( ms.size() * 2 ) );

    for ( size_t i = 0; i < ms.size(); ++i ) {
        a[ i * 2 + 0 ] = ms.getMass( i );
        a[ i * 2 + 1 ] = ms.getIntensity( i );
    }

    WSPutDoubleArray( stdlink, a.get(), dimensions, heads, 2 );
}

