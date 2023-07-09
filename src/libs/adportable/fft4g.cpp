// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "fft4g.hpp"
#include <vector>
#include <complex>
#include <cstdlib>
#include <limits>

extern "C" {
    // void cdft( int, int isgn, double * );
    // void rdft( int, int isgn, double * );
    void cdft(int n, int isgn, double *a, int *ip, double *w);
    void rdft(int n, int isgn, double *a, int *ip, double *w);
}

using namespace adportable;

fft4g::fft4g() : N_(0)
{
}

fft4g::~fft4g()
{
}

void
fft4g::ini( size_t N )
{
    if ( N_ < N ) {
        ip_.resize( 2 + size_t( std::sqrt(N) + 1 ) );
        w_.resize( 1 + N / 2 );
        std::fill( ip_.begin(), ip_.end(), 0 );
        std::fill( w_.begin(), w_.end(), 0 );
    }
}

void
fft4g::cdft( int isgn, std::vector< std::complex< double > >& a )
{
    ini( a.size() );
    ::cdft( a.size() * 2, isgn, reinterpret_cast< double * >(a.data()), ip_.data(), w_.data() );
}

void
fft4g::rdft( int isgn, std::vector< double >& a )
{
    ini( a.size() );
    ::rdft( a.size(), isgn, a.data(), ip_.data(), w_.data() );
}
