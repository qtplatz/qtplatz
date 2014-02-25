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

#include "aminoacid.hpp"
#include <algorithm>
#include <array>

namespace adprot {

    static AminoAcid __table [] = {
        AminoAcid( 'A', "Ala",    "C3H5NO",      "C3H7NO2",    "O=C(O)[C@@H](N)C" )
        , AminoAcid( 'C', "Cys",  "C3H5NOS",     "C3H7NO2S",   "SC[C@H](N)C(O)=O" )
        , AminoAcid( 'D', "Asp",  "C4H5NO3",     "C4H7NO4",    "OC(C[C@H](N)C(O)=O)=O" )
        , AminoAcid( 'E', "Glu",  "C5H7NO3",     "C5H9NO4",    "O=C(O)[C@@H](N)CCC(O)=O" )
        , AminoAcid( 'F', "Phe",  "C9H9NO",      "C9H11NO2",   "N[C@H](C(O)=O)CC1=CC=CC=C1" )
        , AminoAcid( 'G', "Gly",  "C2H3NO",      "C2H5NO2",    "O=C(O)CN" )
        , AminoAcid( 'H', "His",  "C6H7N3O",     "C6H9N3O2",   "N[C@@H](CC1=CN=CN1)C(O)=O" )
        , AminoAcid( 'I', "Ile",  "C6H11NO",     "C6H13NO2",   "O=C(O)[C@@H](N)[C@@H](C)CC" )
        , AminoAcid( 'K', "Lys",  "C6H12N2O",    "C6H14N2O2",  "N[C@H](C(O)=O)CCCCN" )
        , AminoAcid( 'L', "Leu",  "C6H11NO",     "C6H13NO2",   "N[C@@H](CC(C)C)C(O)=O" )
        , AminoAcid( 'M', "Met",  "C5H9NOS",     "C5H11NO2S",  "OC([C@@H](N)CCSC)=O" )
        , AminoAcid( 'N', "Asn",  "C4H6N2O2",    "C4H8N2O3",   "NC(C[C@H](N)C(O)=O)=O" )
        , AminoAcid( 'P', "Pro",  "C5H7NO",      "C5H9NO2",    "O=C([C@@H]1CCCN1)O" )
        , AminoAcid( 'Q', "Gln",  "C5H8N2O2",    "C5H10N2O3",  "OC([C@@H](N)CCC(N)=O)=O" )
        , AminoAcid( 'R', "Arg",  "C6H12N4O",    "C6H14N4O2",  "O=C(O)[C@@H](N)CCCNC(N)=N" )
        , AminoAcid( 'S', "Ser",  "C3H5NO2",     "C3H7NO3",    "OC([C@@H](N)CO)=O" )
        , AminoAcid( 'T', "Thr",  "C4H7NO2",     "C4H9NO3",    "N[C@H](C(O)=O)[C@H](O)C" )
        , AminoAcid( 'V', "Val",  "C5H9NO",      "C5H11NO2",   "N[C@H](C(O)=O)C(C)C" )
        , AminoAcid( 'W', "Trp",  "C11H10N2O",   "C11H12N2O2", "O=C(O)[C@@H](N)CC1=CNC2=C1C=CC=C2" )
        , AminoAcid( 'Y', "Tye",  "C9H9NO2",     "C9H11NO3",   "N[C@@H](CC1=CC=C(O)C=C1)C(O)=O" )
    };
    static int alphabet_index [] = {
        0       // A
        ,-1     // B
        ,1      //'C',
        ,2      //'D',
        ,3      //'E',
        ,4      //'F',
        ,5      //'G',
        ,6      //'H',
        ,7      //'I',
        ,-1     // J
        ,8      //'K',
        ,9      //'L',
        ,10     // 'M',
        ,11     // 'N',
        ,-1     // O
        ,12     // 'P',
        ,13     // 'Q',
        ,14     // 'R',
        ,15     // 'S',
        ,16     // 'T',
        ,-1      // U
        ,17     // 'V',
        ,18     // 'W',
        ,-1      // X
        ,19     // 'Y',
    };
}

# define countof(x) (sizeof(x)/sizeof(x[0]))

using namespace adprot;

AminoAcid::AminoAcid( int symbol_1letter
                      , const char * symbol_3letter
                      , const char * formula
                      , const char * formula_H2O
                      , const char * smiles ) : symbol_1letter_( symbol_1letter )
                                              , symbol_3letter_( symbol_3letter )
                                              , formula_( formula )
                                              , formula_H2O_( formula_H2O )
                                              , smiles_( smiles ) 
{
}

int
AminoAcid::code() const
{
    return symbol_1letter_;
}

const char *
AminoAcid::symbol() const
{
    return symbol_3letter_;
}

const char *
AminoAcid::formula( bool free ) const
{
    if ( free )
        return formula_H2O_;
    else
        return formula_;
}

const char *
AminoAcid::smiles() const
{
    return smiles_;
}

size_t
AminoAcid::size()
{
    return countof( __table );
}

const AminoAcid *
AminoAcid::find( int symbol )
{
    if ( symbol < 'A' || symbol > 'Y' )
        return 0;
    int idx = alphabet_index[ symbol - 'A' ];
    if ( idx < 0 )
        return 0;
    return &__table[ idx ];
}

AminoAcid::iterator
AminoAcid::begin()
{
    return &__table[0];
}

#if defined __GNUC__
# pragma GCC diagnostic ignored "-Warray-bounds"
#endif

AminoAcid::iterator
AminoAcid::end()
{
    return &__table[ countof( __table ) ];
}

