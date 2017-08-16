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

#include "afcolormap.hpp"
#include "cv_extension.hpp"
#include <arrayfire.h>
#include <adportable/debug.hpp>
#include <opencv2/opencv.hpp>
#include <helper_cuda.h>
#include <math.h>

#include <memory>
#include <thread>
#include <mutex>

//using namespace video::af;

std::unique_ptr< af::Window > wnd;
std::once_flag flag;

void
afApplyColorMap( const cv::Mat& src, cv::Mat& dst, float scale )
{
    std::call_once( flag, [](){
            wnd = std::make_unique< af::Window >();
            wnd->grid( 1, 2 );
        } );
    bool transpose = true;
    const int channels = src.channels();

    af::array rgb;
    if ( src.channels() == 1 && src.type() == CV_32FC(1) ) {
        af::array a = af::array( src.cols, src.rows, 1, src.ptr< float >( 0 ) ).T();
        rgb = af::gray2rgb( a * scale );
    } else if ( src.channels() == 3 && src.type() == CV_8UC(3) ) {
        rgb = af::array( src.cols, src.rows, channels, src.ptr< uint8_t >( 0 ) ).T();
        // af::array ind = af::array( af::seq( channels - 1, channels, w * channels - 1) );
        // rgb = af::array( h, w, channels );
        // gfor( af::array k, channels) {
        //     rgb( af::span, af::span, k ) = t(ind - k, af::span ); //.T();
        // }
    } else {
        return;
    }
    
    (*wnd)(0, 0).image( rgb, "rgb" );

    af::array rgb_t = rgb.T();
    (*wnd)(0, 1).image( rgb_t, "rgb_t" );

    wnd->show();
    
    dst = cv::Mat( src.rows, src.cols, CV_8UC(3) ); //  video::cv_extension::mat_t< uint8_t, 3 >::type_value );    
    rgb_t.as( u8 ).host( dst.ptr< uchar >( 0 ) );
}

