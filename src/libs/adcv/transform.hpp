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

namespace af { class array; }
namespace cv { class Mat; }
class QImage;

namespace adcv {

    // cv::Mat uses BGR format, which does not handle in this transform class
    // use bgr2rgb_ template instead

    class ADCVSHARED_EXPORT transform {
    public:
        static cv::Mat mat( const af::array& );
        static af::array array( const cv::Mat& );
    };

    template< typename T >
    struct ADCVSHARED_EXPORT transform_ {
        template< typename R > T operator()( const R& ) const;
    };

    template<>
    template< typename R > af::array ADCVSHARED_EXPORT transform_< af::array >::operator()( const R& ) const;

    template<>
    template< typename R > cv::Mat ADCVSHARED_EXPORT transform_< cv::Mat >::operator()( const R& ) const;

    template<>
    template< typename R > QImage ADCVSHARED_EXPORT transform_< QImage >::operator()( const R& ) const;

}
