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
#if HAVE_ARRAYFIRE
# include <af/array.h>
#endif
#include <opencv2/core/mat.hpp>
#include <boost/numeric/ublas/fwd.hpp>
#include <vector>
#include <memory>

class QImage;

namespace advision {

    enum cuda_algo { cuda_none, cuda_arrayfire, cuda_direct };

    class ADVISIONSHARED_EXPORT ApplyColorMap {
    protected:
        std::vector< float > levels_;
        std::vector< float > colors_;
    public:
        ~ApplyColorMap();
        ApplyColorMap();
        ApplyColorMap( size_t nlevels, const float * levels, const float * colors );

        cv::Mat operator()( const cv::Mat&, float scaleFactor = 1.0, cuda_algo algo = cuda_direct ) const;
    };

    //ADVISIONSHARED_TEMPLATE_EXPORT 
	template< typename T >
    struct ADVISIONSHARED_EXPORT ApplyColorMap_ : public ApplyColorMap {
        ApplyColorMap_() : ApplyColorMap() {}
        ApplyColorMap_( size_t nlevels, const float * levels, const float * colors ) : ApplyColorMap( nlevels, levels, colors ) {}

        // [1]
        template< typename R > T operator()( const boost::numeric::ublas::matrix< R >&, float scaleFactor = 1.0 ) const;

        // [2]
        template< typename R > T operator()( const R&, float scaleFactor = 1.0 ) const;
    };

    // specialization [1]
    template<>
    template<typename R> QImage ApplyColorMap_<QImage>::operator()( const boost::numeric::ublas::matrix< R >&, float scaleFactor ) const;

#if HAVE_OPENCV
    // specialization [2]
    template<>
    template<typename R> cv::Mat ApplyColorMap_<cv::Mat>::operator()( const R&, float scaleFactor ) const;
#endif

#if HAVE_ARRAYFIRE
    template<>
    template<typename R> af::array ApplyColorMap_<af::array>::operator()( const R&, float scaleFactor ) const;
#endif    
} // namespace advision












