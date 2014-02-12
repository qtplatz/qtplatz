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

#include "isotope.hpp"
#include "molecule.hpp"
#include "tableofelement.hpp"
#include <string>

isotope::isotope()
{
}

#if 0
static int
calculate_peaks( std::vector< isotope >& peaks )
{
    peaks.clear();
    for( compound& c: mol ) {
        for( int k = 0; k < c.natoms(); k++ ) {
            for ( const isotope& i: c.isotopes() ) {
                if ( k == 0 ) {
                    peaks.push_back( isotope( i.mass(), i.abundance() ) );
                } else {
                    for ( auto& p: peaks ) {
                        isotope np1( p.mass() + i.mass(), p.abundance() * i.abundance() );
                        auto it = std::lower_bound( peaks.begin(), peaks.end(), np1
                                                    , [](const isotope& lhs, const isotope& rhs){
                                                        return lhs.mass() < rhs.mass();
                                                    });
                        peaks.insert( it, np1 );
                    }
                }
            }
        }
    }
    return 1;
}
#endif

void
isotope::append( molecule& mol, const molecule::isotope& pk )
{
    // todo: sort by mass, and marge if delta mass is less than threshold
    mol.isotopes.push_back( pk );
}

bool
isotope::compute( molecule& mol )
{
    mol.isotopes.clear();

    std::vector< std::pair< tableofelement::element, size_t > > xmol;
    for ( auto& e: mol.elements ) {
        tableofelement::element toe = tableofelement::findElement( e.symbol );
        xmol.push_back( std::make_pair( toe, e.count ) );
    }

    for ( auto& c: xmol ) {
        const tableofelement::element& toe = c.first;
        size_t count = c.second;
        for( size_t k = 0; k < count; k++ ) {
            for ( const tableofelement::isotope& i: toe.isotopes() ) {
                if ( k == 0 ) {
                    mol.isotopes.push_back( molecule::isotope( i.mass, i.abundant ) );
                } else {
                    for ( auto& pk: mol.isotopes ) {
                        molecule::isotope npk( pk.mass + i.mass, pk.abundance * i.abundant );
                        append( mol, npk );
                        //mol.isotopes.push_back( npk );
                    }
                }
            }
        }
    }

    return true;
}
