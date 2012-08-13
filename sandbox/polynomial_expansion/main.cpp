/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include <QCoreApplication>

#include <algorithm>
#include <string>
#include <vector>
#include <iostream>
#include <adportable/combination.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adportable/array_wrapper.hpp>
#include <numeric>
#include <iostream>
#include <boost/utility.hpp>
#include <boost/math/special_functions/factorials.hpp>

#  if defined NDEBUG
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "adportable.lib")
#  else
#     pragma comment(lib, "../../lib/qtplatz/adcontrolsd.lib")
#  endif

struct term {
    int index;
    int factor;
	int var;
	term( int _var ) : index(1), factor(1), var( _var ) {}
	term( const term& t ) : index(t.index), factor(t.factor), var( t.var ) {}
	const term& operator = ( const term& t ) {
        index = t.index;
		factor = t.factor;
		var = t.var;
		return *this;
	}
};

struct partial_molecular_mass {
	static double combination( size_t n, size_t r ) {
		double a = n;
		if ( r > 0 ) {
			size_t x = n; 
			while ( --x > ( n - r ) )
				a *= x;
			do {
				a /= r;
			} while( --r );
			return a;
		}
		return 0;
	}

	static double factorial( size_t n ) {
        double d = n;
		while ( n-- != 1 )
			d = d * n;
		return d;
	}
};

int
main(int argc, char *argv[])
{
	//QCoreApplication a(argc, argv);
	//return a.exec();
   
	std::vector< term > terms;
    terms.push_back( term( 'x' ) );
    terms.push_back( term( 'y' ) );
	// binomial theorem
    // (x + y)^n = x^k * y^(n - k)
    // (n,k) = nCk
	// double nCk = boost::n! / ( n - k )! * k!;
    int nterm = 100;
	for ( int k = 1; k <= nterm; ++k ) {
		double nCk = partial_molecular_mass::factorial( nterm ) / 
			( partial_molecular_mass::factorial( nterm - k ) * partial_molecular_mass::factorial( k ) );
		std::cout << nterm << ", " << k << ", " << nCk << ", " << partial_molecular_mass::combination( nterm, k ) << std::endl;
	}

    return 0;
}
