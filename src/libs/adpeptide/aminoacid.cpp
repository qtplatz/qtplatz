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

#include "aminoacid.hpp"
#include <algorithm>
#include <array>

namespace adpeptide {

    struct AA {
        const char _1letter_symbol;
        const char * _3letter_symbol;
        const char * formula;
        const char * smiles;
    };
    
    static AA protein_compositions [] = {
        { 'A', "Ala", "C3H7NO2",      "O=C(O)[C@@H](N)C" }
        , { 'C', "Cys", "C3H7NO2S",   "SC[C@H](N)C(O)=O" }
        , { 'D', "Asp", "C4H7NO4",    "OC(C[C@H](N)C(O)=O)=O" }
        , { 'E', "Glu", "C5H9NO4",    "O=C(O)[C@@H](N)CCC(O)=O" }
        , { 'F', "Phe", "C9H11NO2",   "N[C@H](C(O)=O)CC1=CC=CC=C1" }
        , { 'G', "Gly", "C2H5NO2",    "O=C(O)CN" }
        , { 'H', "His", "C6H9N3O2",   "N[C@@H](CC1=CN=CN1)C(O)=O" }
        , { 'I', "Ile", "C6H13NO2",   "O=C(O)[C@@H](N)[C@@H](C)CC" }
        , { 'K', "Lye", "C6H14N2O2",  "N[C@H](C(O)=O)CCCCN" }
        , { 'L', "Leu", "C6H13NO2",   "N[C@@H](CC(C)C)C(O)=O" }
        , { 'M', "Met", "C5H11NO2S",  "OC([C@@H](N)CCSC)=O" }
        , { 'N', "Asn", "C4H8N2O3",   "NC(C[C@H](N)C(O)=O)=O" }
        , { 'P', "Pro", "C5H9NO2",    "O=C([C@@H]1CCCN1)O" }
        , { 'Q', "Gln", "C5H10N2O3",  "OC([C@@H](N)CCC(N)=O)=O" }
        , { 'R', "Arg", "C6H14N4O2",  "O=C(O)[C@@H](N)CCCNC(N)=N" }
        , { 'S', "Ser", "C3H7NO3",    "OC([C@@H](N)CO)=O" }
        , { 'T', "Thr", "C4H9NO3",    "N[C@H](C(O)=O)[C@H](O)C" }
        , { 'V', "Val", "C5H11NO2",   "N[C@H](C(O)=O)C(C)C" }
        , { 'W', "Trp", "C11H12N2O2", "O=C(O)[C@@H](N)CC1=CNC2=C1C=CC=C2" }
        , { 'Y', "Tye", "C9H11NO3",   "N[C@@H](CC1=CC=C(O)C=C1)C(O)=O" }
    };

    static AminoAcid __table [] = {
        AminoAcid( 'A' )
        , AminoAcid( 'C' )
/*
        , { 'D' }
        , { 'E' }
        , { 'F' }
        , { 'G' }
        , { 'H' }
        , { 'I' }
        , { 'K' }
        , { 'L' }
        , { 'M' }
        , { 'N' }
        , { 'P' }
        , { 'Q' }
        , { 'R' }
        , { 'S' }
        , { 'T' }
        , { 'V' }
        , { 'W' }
        , { 'Y' }
*/
    };
}

#define countof(x) (sizeof(x)/sizeof(x[0]))

using namespace adpeptide;

AminoAcid::AminoAcid( char symbol ) : symbol_( symbol )
{
    if ( symbol >= countof( protein_compositions ) )
        throw std::exception();

    if ( protein_compositions[ symbol ]._3letter_symbol == 0 )
        throw std::exception();        
}

AminoAcid::AminoAcid( const char * symbol ) : symbol_(0)
{
    for ( auto& aa: protein_compositions ) {
        if ( std::strcmp( symbol, aa._3letter_symbol ) == 0 ) {
			symbol_ = aa._1letter_symbol;
            break;
        }
    }
}

const AminoAcid::iterator
AminoAcid::begin()
{
    return AminoAcid::iterator( 0 );
}

const AminoAcid::iterator
AminoAcid::end()
{
    return AminoAcid::iterator( countof( protein_compositions ) );
}

AminoAcid::iterator::iterator( size_t pos ) : pos_( pos )
{
}

AminoAcid::iterator::operator AminoAcid* () const 
{
    return &__table[pos_];
}

