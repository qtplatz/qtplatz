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
/*
 * FFT and related algolism for spectral peak picking
 * Originally created by T. Hondo, '90, 01/09
 */

#pragma once

#include <vector>
#include <complex>
#include <cstdlib>
#include <limits>
#include "adportable_global.h"

namespace adportable {

    class ADPORTABLESHARED_EXPORT fft4g;

	class fft4g {
        std::vector< int > ip_;  // 2 + (1<< int( (log(n)+0.5)/log(2))/2) := sqrt(N)
        std::vector< double > w_; // N/2
        size_t N_;
        void ini( size_t N );
	public:
        ~fft4g();
        fft4g();
        void cdft( int isgn, std::vector< std::complex< double > >& );
        void rdft( int isgn, std::vector< double >& );
	};

}
