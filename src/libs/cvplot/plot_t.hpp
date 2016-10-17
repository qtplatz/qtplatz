/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

namespace cvplot {

    template< cv::CvType = CV_8UC3 >
    struct make_mat {

        template typename< Matrix > cv::Mat operator( const Matrix& matrix ) const {

            cv::Mat m( matrix.size1(), matrix.size2(), CV_8UC3 ); // BGR
            
            for ( size_t i = 0; i < matrix.size1(); ++i ) {
                for ( size_t j = 0; j < matrix.size2(); ++j ) {
                    auto t = matrix( i, j );
                    uint8_t r = ( ( t & 0700 ) >> 6 ) * 255;
                    uint8_t g = ( ( t & 0070 ) >> 3 ) * 255;
                    uint8_t b = ( ( t & 0007 ) >> 0 ) * 255;
                    m.at< cv::Vec3b >( i, j )[0] = r;
                    m.at< cv::Vec3b >( i, j )[1] = g;
                    m.at< cv::Vec3b >( i, j )[2] = b;
                }
            }

            return m;
        }

        
    };

}
        
                
            
