// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <compiler/disable_sign_compare.h>
#include <compiler/disable_unused_parameter.h>
#include <compiler/disable_unused_function.h>

#include "polfit.hpp"
#include <cmath>
#include <limits>
#include <cstring>
#include "float.hpp"
# if defined _MSC_VER
# pragma warning(disable: 4996)
# endif
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
# if defined _MSC_VER
# endif

namespace adportable {

    namespace internal {
        static double determ( boost::numeric::ublas::matrix<double>& a, int n );
        static double weight_value( polfit::WeightingType wmode, double yi );
        //----------------------------
        static void normalCoefficients( int count
                                       , const double* x, const double* y, int degree
                                       , boost::numeric::ublas::vector<double>& xSum
                                       , boost::numeric::ublas::vector<double>& yxSum
                                       , boost::numeric::ublas::matrix<double>& a
                                       , boost::numeric::ublas::vector<double>& b );
	
        static void normalCoefficients( int count
                                      , const double* x, const double * y
                                      , int degree
                                      , boost::numeric::ublas::matrix<double>& a
                                      , boost::numeric::ublas::vector<double>& b );
    }
}

using namespace adportable;

/*
 * Reference for polfit() and determ();
 *  Pages 140-, 293-
 *  Data Reduction and Error Analysis for the Physical Sciences.
 *  Philip R. Bevington, McGraw-Hill Book Company, 1969 
*/

/*
 * Calculate the determinant of a square matrix
 * det = determ(array, norder);
 * array - matrix
 * norder - order of determinant (degree of matrix)
*/
//==============================================================================
// Recursive definition of determinate using expansion by minors.
//
// Notes: 1) arguments:
//             a (double **) pointer to a pointer of an arbitrary square matrix
//             n (int) dimension of the square matrix
//
//        2) Determinant is a recursive function, calling itself repeatedly
//           each time with a sub-matrix of the original till a terminal
//           2X2 matrix is achieved and a simple determinat can be computed.
//           As the recursion works backwards, cumulative determinants are
//           found till untimately, the final determinate is returned to the
//           initial function caller.
//
//        3) m is a matrix (4X4 in example)  and m13 is a minor of it.
//           A minor of m is a 3X3 in which a row and column of values
//           had been excluded.   Another minor of the submartix is also
//           possible etc.
//             m  a b c d   m13 . . . .
//                e f g h       e f . h     row 1 column 3 is elminated
//                i j k l       i j . l     creating a 3 X 3 sub martix
//                m n o p       m n . p
//
//        4) the following function finds the determinant of a matrix
//           by recursively minor-ing a row and column, each time reducing
//           the sub-matrix by one row/column.  When a 2X2 matrix is
//           obtained, the determinat is a simple calculation and the
//           process of unstacking previous recursive calls begins.
//
//                m n
//                o p  determinant = m*p - n*o
//
//        5) this function uses dynamic memory allocation on each call to
//           build a m X m matrix  this requires **  and * pointer variables
//           First memory allocation is ** and gets space for a list of other
//           pointers filled in by the second call to malloc.
//
//        6) C++ implements two dimensional arrays as an array of arrays
//           thus two dynamic malloc's are needed and have corresponsing
//           free() calles.
//
//        7) the final determinant value is the sum of sub determinants
//
//==============================================================================


double 
internal::determ( boost::numeric::ublas::matrix<double>& a, int n ) // double **a, int n)
{
    int i,j,j1,j2;
    double det = 0;
    // double ** m = 0;
    boost::numeric::ublas::matrix< double > m( n, n );

    if (n < 1) { 
        /* Error */
    } else if (n == 1) { /* Shouldn't get used */
        det = a(0, 0); //[0][0];
    } else if (n == 2) {
        det = a(0, 0) * a(1, 1) - a(1, 0) * a(0, 1);
    } else {
        det = 0;
        for (j1 = 0; j1 < n ; j1++) {
            for ( i = 1; i < n; i++) {
                j2 = 0;
                for (j = 0; j < n; j++) {
                    if (j == j1)
                        continue;
                    m( i-1, j2 ) = a(i, j);
                    j2++;
                }
            }
            det += pow(-1.0, 1.0 + j1 + 1.0) * a(0, j1) * determ(m, n - 1);
        }
    }
    return det;
}


int 
polfit::fit( const double *x
            , const double *y
            , int npts
            , int nterms
            , std::vector<double>& polynomial
            , double & chisqr
            , WeightingType wmode )
{
    int i, j, k, l, n;
    const int nmax = 2 * nterms - 1;
    std::vector<double> sumx(nmax), sumy(nterms);
    boost::numeric::ublas::matrix< double > array( nterms, nterms );    

    polynomial.clear(); 

    memset( &sumx[0], 0, sizeof(double) * sumx.size() );
    memset( &sumy[0], 0, sizeof(double) * sumy.size() );
    chisqr = 0;
    double chisq = 0.0;
    double xi, yi, wt, xterm, yterm, delta, xfree;
    for (i = 0; i < npts; ++i) { // DO 50
        xi = x[i];
        yi = y[i];
        wt = internal::weight_value( wmode, yi );

        xterm = wt;
        // DO 44 N + 1, NMAX
        for (n = 0; n < nmax; ++n) {
            sumx[n] = sumx[n] + xterm;
            xterm = xterm * xi;
        }
        // DO 48 N=1, NTERMS
        yterm = wt * yi;
        for (n = 0; n < nterms; ++n) {
            sumy[n] = sumy[n] + yterm;
            yterm = yterm * xi;
        }
        chisq = chisq + wt * (yi * yi);
    }//L50:

    /* L51 */
    for (j = 0; j < nterms; ++j) {
        for (k = 0; k < nterms; ++k) {
            n = j + k; // - 1;
            array(j, k) = sumx[n]; //[j][k] = sumx[n];
        }
    }
    delta = internal::determ(array, nterms);
    if (delta == 0) {
        chisqr = 0;
        return 0;
    } else {
        for (l = 0; l < nterms; ++l) {
            for (j = 0; j < nterms; ++j) {
                for (k = 0; k < nterms; ++k) {
                    n = j + k;
                    array(j, k) = sumx[n];
                }
                array(j, l) = sumy[j];
            }
            polynomial.push_back( internal::determ( array, nterms ) / delta );
        }
    }
    // DO 54 J=1, NTERMS
    for (j = 0; j < nterms; ++j) {
        chisq = chisq - 2.0 * polynomial[j] * sumy[j];
        for (k = 0; k < nterms; ++k) {
            n = j + k; // - 1;
            chisq = chisq + polynomial[j] * polynomial[k] * sumx[n];
        }
    }
    xfree = npts - nterms;
    chisqr = chisq / xfree;
    return 0;
}

double
internal::weight_value( polfit::WeightingType wmode, double yi )
{
    double wt( 1.0 );
    switch(wmode) {
    case polfit::WEIGHTING_NONE:
        wt = 1.0;
        break;
    case polfit::WEIGHTING_INV_C:
        wt = 1.0 / std::abs(yi);
        break;
    case polfit::WEIGHTING_INV_C2:
        wt = 1.0 / (yi * yi);
        break;
    case polfit::WEIGHTING_INV_C3:
        wt = 1.0 / std::abs(yi * yi * yi);
        break;
    }
    if ( compare<double>::approximatelyEqual(wt, 0.0) )
        wt = 1.0;
    return wt;
}


polfit::polfit()
{
}

////////////////////// 
// Calculate n + 1 equeations from given x, y data
// Compute the coefficients for these equations
/////

void
internal::normalCoefficients( int npts
                           , const double * xArray
                           , const double * yArray 
                           , int degree	// degree of polynomial 
                           , boost::numeric::ublas::vector<double>& xSum
                           , boost::numeric::ublas::vector<double>& yxSum
                           , boost::numeric::ublas::matrix<double>& a			// 2 dimentional array for the a's.
                           , boost::numeric::ublas::vector<double>& b )		// 1 dimentional array for the b's, the right hand side.
{
	long xCount	= degree * 2 + 1;	// 2n
	long n		= degree + 1;		// n + 1 equations

	long i;
	for ( i = 0; i < xCount; i++ )
		xSum[i]	= 0.0;

	for ( i = 0; i < n; i++ )
		yxSum[i] = 0.0;

	// Coumpute the sums.
	for ( i = 0; i < npts; i++ )	{
		double x = xArray[i];
		double y = yArray[i];
		
		double lastx = 1.0;
		int j;
        for ( j = 0; j < n; j++ )		{
			xSum[j] += lastx;
			yxSum[j] += y * lastx;
			lastx *= x;
		}

		for ( ; j < xCount; j++ )		{
			xSum[j] += lastx;
			lastx *= x;
		}
	}

	// Fill out cofficients matrix.
	for ( i = 0; i < n; i++ )	{
        for ( long j = 0; j < n; j++ )
			a(i,j) = xSum[j+i];
		b[i] = yxSum[i];
	}
}

void
internal::normalCoefficients(int count
                           , const double* xArray
                           , const double* yArray 
                           , int degree	// degree of polynomial  
                           , boost::numeric::ublas::matrix<double>& a			// 2 dimentional array for the a's.
                           , boost::numeric::ublas::vector<double>& b )		// 1 dimentional array for the b's, the right hand side.
{
	long xCount	= degree * 2 + 1;	// 2n
	long n		= degree + 1;		// n + 1 equations

    boost::numeric::ublas::vector<double> xSum( xCount );
    boost::numeric::ublas::vector<double> yxSum( n );

    normalCoefficients( count, xArray, yArray, degree, xSum, yxSum, a, b );
}

////////////

bool
polfit::fit( const double* x, const double* y, int npts, int nterms, std::vector<double>& coeffs )
{
    if ( nterms > npts )
        return false;

    boost::numeric::ublas::vector< int > index( nterms );
    boost::numeric::ublas::matrix<double> a( nterms, nterms );
    boost::numeric::ublas::vector<double> b( nterms );

    internal::normalCoefficients( npts, x, y, nterms - 1, a, b );

    boost::numeric::ublas::permutation_matrix<> pm( a.size1() );
    boost::numeric::ublas::lu_factorize<>( a, pm );
    boost::numeric::ublas::lu_substitute<>( a, pm, b );

    coeffs.clear();
    for ( int i = 0; i < nterms; ++i )
        coeffs.push_back( b[i] );
	
	return true;
}

