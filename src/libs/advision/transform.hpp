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

namespace af { class array; }
namespace cv { class Mat; }

namespace advision {

    // cv::Mat uses BGR format, which does not handle in this transform class

    class ADVISIONSHARED_EXPORT transform {
    public:
        static cv::Mat mat( const af::array& );
        static af::array array( const cv::Mat& );
    };

    template< typename T >
    struct ADVISIONSHARED_EXPORT transform_ {
        template< typename M > T operator()( const M& ) const;
        T bgr2rgb( const T& ) const;
    };

}












