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

#include "bgr2rgb.hpp"
#include "transform.hpp"
#if HAVE_ARRAYFIRE
# include <arrayfire.h>
#endif
#include <opencv2/opencv.hpp>


namespace advision {

#if HAVE_ARRAYFIRE
    template<>
    template<>
    af::array bgr2rgb_< af::array >::operator()< af::array >( const af::array& a ) const {
        af::array b( a );
        b( af::span, af::span, 2 ) = a( af::span, af::span, 0 );
        b( af::span, af::span, 0 ) = a( af::span, af::span, 2 );
        return b;
    }

    template<>
    template<>
    cv::Mat bgr2rgb_< cv::Mat >::operator()< af::array >( const af::array& a ) const {
        auto t = bgr2rgb_< af::array >()( a );
        return transform_< cv::Mat >()( t );
    }
#endif

    template<>
    template<>
    cv::Mat bgr2rgb_< cv::Mat >::operator()< cv::Mat >( const cv::Mat& a ) const {
        cv::Mat b;
        cv::cvtColor( a, b, CV_RGB2BGR );
        return b;
    }
    
}

