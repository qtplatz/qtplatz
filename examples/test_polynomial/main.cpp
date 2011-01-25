// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include <QtCore/QCoreApplication>
#include <adportable/polfit.h>
#include <cmath>
#include <boost/numeric/ublas/matrix.hpp>

struct polynomial {
    std::vector<double> term_;
    const size_t n_;
    polynomial( size_t nTerm, std::vector<double> term ) : n_( nTerm ), term_( term ) {
    }
    double operator () ( double x ) const {
        double y = term_[0];
        for ( size_t i = 1; i < n_; ++i )
            y += term_[i] * std::pow( x, int(i) );
        return y;
    }
};

int
main(int argc, char *argv[])
{
    Q_UNUSED( argc );
    Q_UNUSED( argv );

    std::vector<double> terms;
    for ( int i = 0; i < 30; ++i )
        terms.push_back( 1 );


    for ( size_t nTerm = 2; nTerm <= 13; ++nTerm ) {

        const size_t NPTS = nTerm * 2;

        polynomial func2( nTerm, terms );

        std::vector<double> x;
        std::vector<double> y;
        for ( int i = 0; i < NPTS; ++i ) {
            x.push_back( i );
            y.push_back( func2( x[i] ) );
        }

        std::vector<double> rterms1, rterms2;
        double chisqr1;
        //adportable::polfit::compute( &x[0], &y[0], NPTS, nTerm, rterms1, chisqr1 );
        adportable::polfit::fit( &x[0], &y[0], NPTS, nTerm, rterms2 );

        polynomial estimate( nTerm, rterms2 );
        double sumXX = 0;
        for ( int i = 0; i < NPTS; ++i ) {
            double dy = estimate( x[i] ) - y[i]; 
            sumXX = dy * dy;
        }
        double sd = sumXX / NPTS;

        long debug  = 0;

    }
}