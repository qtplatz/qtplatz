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

#include <opencv2/core/core.hpp>
#include <QColor>
#include <memory>

namespace adcontrols { class MappedImage; }

namespace video {

    class cvmat {
    public:
        cv::Mat operator()( const adcontrols::MappedImage& ) const;
        std::shared_ptr< adcontrols::MappedImage > operator()( const cv::Mat&, size_t ) const;
        void mesh( cv::Mat&, size_t split, size_t width ) const;
        static cv::Mat scaleLog( const cv::Mat& );
    };

    class cvColor {
        struct Color {
            double r; double g; double b; double value;
            Color(double _r, double _g, double _b, double _v) : r(_r), g(_g), b(_b), value(_v) {}
        };
        std::vector< Color > colors_;
    public:
        cvColor( bool gray = false );
        QColor color( double ) const;
        cv::Mat operator()( const adcontrols::MappedImage&, int z ) const;
        cv::Mat operator()( const cv::Mat& /* CV_32F */, double scale = 1.0 ) const;
    };
    
}

