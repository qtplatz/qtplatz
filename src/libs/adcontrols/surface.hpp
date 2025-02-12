// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC
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

#pragma once

#include "adcontrols_global.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <memory>

namespace adcontrols {

    class MassSpectrum;

    class Surface {
        std::vector< std::shared_ptr< adcontrols::MassSpectrum > > data_;
        std::vector< double > xValues_;
        std::vector< double > yValues_;
    public:
        Surface();
        void operator << ( std::shared_ptr< adcontrols::MassSpectrum >&& );

        boost::numeric::ublas::matrix<double> getSurface() const;
        std::pair< size_t, size_t > size() const;
        const std::vector< double >& yValues() const;
        const std::vector< double >& xValues() const;
    };
}
