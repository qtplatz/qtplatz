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

#include "advision_global.hpp"
#include <af/array.h>
#include <opencv2/core/mat.hpp>
#include <vector>
#include <memory>

namespace advision {

    namespace cpu { class ColorMap; }
    namespace gpu_af { class ColorMap; }
    namespace gpu_cv { class ColorMap; }

    enum cuda_algo { cuda_none, cuda_arrayfire, cuda_direct };
    
    class ADVISIONSHARED_EXPORT ApplyColorMap {
        std::vector< float > levels_;
        std::vector< float > colors_;
    public:
        ~ApplyColorMap();
        ApplyColorMap();
        ApplyColorMap( size_t nlevels, const float * levels, const float * colors );
        
        cv::Mat operator()( const cv::Mat&, float scaleFactor = 1.0, cuda_algo algo = cuda_direct ) const;
#if HAVE_ARRAYFIRE        
        af::array operator()( const af::array&, float scaleFactor = 1.0 ) const;
#endif
    };

} // namespace advision












