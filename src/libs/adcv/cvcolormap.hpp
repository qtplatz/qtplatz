/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "adcv_global.hpp"
#include <driver_types.h> // cudaStream_t
#include <vector>
#include <thrust/device_vector.h>
#include <boost/numeric/ublas/fwd.hpp>

namespace cv { class Mat; }

class QImage;

namespace cuda {

    class ADCVSHARED_EXPORT cvColorMap {

        cvColorMap( const cvColorMap& ) = delete;
        cvColorMap& operator = ( const cvColorMap& ) = delete;

        thrust::device_vector< float > d_levels_;
        thrust::device_vector< float > d_colors_;

    public:
        cvColorMap( const std::vector< float >& levels, const std::vector< float >& colors );
        ~cvColorMap();

        cv::Mat operator()( const cv::Mat&, float scaleFactor = 1.0 ) const;
        cv::Mat operator()( const boost::numeric::ublas::matrix<double>&, float scaleFactor = 1.0 ) const;
    };
}
