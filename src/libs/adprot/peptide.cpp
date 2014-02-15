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

#include "peptide.hpp"
#include "aminoacid.hpp"
#include <algorithm>

using namespace adprot;

peptide::peptide() : mass_( 0 )
{
}

peptide::peptide( const peptide& t ) : annotation_( t.annotation_ )
                                     , sequence_( t.sequence_ )
                                     , formula_( t.formula_ )
                                     , mass_( t.mass_ )
{
}

peptide::peptide( const std::string& sequence
                  , const std::string& formula
                  , double mass ) : sequence_( sequence )
                                  , formula_( formula )
                                  , mass_( mass )
{
}

const std::string&
peptide::annotation() const
{
    return annotation_;
}

void
peptide::setAnnotation( const std::string& value )
{
    annotation_ = value;
}

const std::string&
peptide::sequence() const
{
    return sequence_;
}

void
peptide::setSequence( const std::string& sequence )
{
    sequence_ = sequence;
}

double
peptide::mass() const
{
    return mass_;
}

void
peptide::setMass( const double& value )
{
    mass_ = value;
}

const std::string&
peptide::formula() const
{
    return formula_;
}

void
peptide::setFormula( const std::string& value )
{
    formula_ = value;
}

// static
std::string
peptide::formula( const std::string& sequence )
{
    std::string formula;

    for( char a: sequence ) {
        if ( const AminoAcid * aa = AminoAcid::find( int(a) ) )
            formula += aa->formula( false );
    }
    return formula + "H2O";
}


