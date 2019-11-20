// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2019-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019-2020 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

//#include "dataprocessor.hpp"
//#include "datareader.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isotopecluster.hpp>
// #include <adcontrols/datafile.hpp>
// #include <adcontrols/datareader.hpp>
// #include <adportable/debug.hpp>
// #include <adcontrols/datareader.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adportable/debug.hpp>
#include <codecvt>
#include <memory>

// #include <boost/uuid/uuid_io.hpp>
// #include <boost/uuid/uuid_generators.hpp>

#include "wstp.h"

int
main( int argc, char * argv[] )
{
    ADDEBUG() << "main argc=" << argc << "\n" << argv[0];

    return WSMain( argc, argv );
}

int
addtwo( int i, int j )
{
    ADDEBUG() << "addtwo(" << i << ", " << j << ")";
	return i+j;
}

int
counter( int i )
{
    static int __counter;
    __counter += i;
    return __counter;
}

double
monoIsotopicMass( const char * formula )
{
    ADDEBUG() << __FUNCTION__ << "(" << formula << ")";
    double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( adcontrols::ChemicalFormula::split( formula ) );
    return exactMass;
}

void
isotopeCluster( const char * formula, double resolving_power )
{
    ADDEBUG() << __FUNCTION__ << "(" << formula << ", " << resolving_power << ")";

    adcontrols::MassSpectrum ms;

    adcontrols::isotopeCluster()( ms, formula, 1.0, resolving_power );

    long dimensions[ 2 ] = { long(ms.size()), 2 };
    const char * heads[ 2 ] = { "List", "List" };
    std::unique_ptr< double[] > a( std::make_unique< double[] >( ms.size() * 2 ) );

    for ( size_t i = 0; i < ms.size(); ++i ) {
        a[ i * 2 + 0 ] = ms.mass( i );
        a[ i * 2 + 1 ] = ms.intensity( i );
        ADDEBUG() << a[ i * 2 + 0 ] << ", " << a[ i * 2 + 1 ];
    }

    WSPutDoubleArray( stdlink, a.get(), dimensions, heads, 2 );
}

int
adFileOpen( const char * name )
{
    ADDEBUG() << __FUNCTION__ << "(" << name << ")";
    return 99;
}
