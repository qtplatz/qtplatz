/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
**************************************************************************/

#pragma once

#include "advision_global.hpp"
#include <boost/numeric/ublas/fwd.hpp>
#include <limits>

namespace advision {

    class Moments {
        std::pair< int, int > blurCount_;
        std::pair< unsigned, unsigned > szThreshold_;
        int sizeFactor_;
        int cannyThreshold_;
    public:
        Moments( const std::pair<int,int>& blurCount = {0,0}, int sizeFactor = 1, int cannyThreshold = 0
                 , const std::pair<unsigned, unsigned>& szThreshold = { 0, std::numeric_limits<unsigned>::max()} );
        Moments( const Moments& );

        boost::numeric::ublas::matrix< double > operator()( const boost::numeric::ublas::matrix< double >& ) const;
        void operator()( boost::numeric::ublas::matrix< double >& moments, const boost::numeric::ublas::matrix< double >& ) const;
    };

}
