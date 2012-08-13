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
    int nterm = 2;
    for ( int k = 1; k <= nterm; ++k ) 
		std::cout << nterm << ", " << k << ", " << boost::math::factorial( nterm ) << std::endl;

    return 0;
}
