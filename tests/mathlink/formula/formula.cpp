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

extern "C" {
    double monoIsotopicMass( const char * );
    void standardFormula( const char * );
    void isotopeCluster( const char *, double resolution );
}

double
monoIsotopicMass( const char * formula )
{
    return adcontrols::ChemicalFormula().getMonoIsotopicMass( formula );
}

void
standardFormula( const char * formula )
{
    auto result = adcontrols::ChemicalFormula::standardFormula( formula );
    WSPutString( stdlink, result.c_str() );
}

void
isotopeCluster( const char * formula, double rp )
{
    adcontrols::isotopeCluster cluster;
    cluster.threshold_daltons( 1.0 / rp );

    std::vector< adcontrols::isotopeCluster::isopeak > peaks;
    cluster( peaks, formula );

    long dimensions[ 2 ] = { long(peaks.size()), 2 };
    const char * heads[ 2 ] = { "List", "List" };
    std::unique_ptr< double[] > a( std::make_unique< double[] >( peaks.size() * 2 ) );

    for ( size_t i = 0; i < peaks.size(); ++i ) {
        a[ i * 2 + 0 ] = peaks[i].mass;
        a[ i * 2 + 1 ] = peaks[i].abundance;
    }

    WSPutDoubleArray( stdlink, a.get(), dimensions, heads, 2 );
}

