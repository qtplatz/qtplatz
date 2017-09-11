/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "advision_global.hpp"
#include <boost/numeric/ublas/fwd.hpp>
#include <adcontrols/contoursmethod.hpp>
#include <tuple>
#include <limits>

class QImage;

namespace advision {

    struct imGrayScale {};
    struct imRGBColor {};
    struct imColorMap {};
    struct imBlur { std::pair<int, int> size; imBlur( int x = 5, int y = 5 ) : size( x, y ) {} };
    struct imDFT {};
    struct imContours : public adcontrols::ContoursMethod {
        imContours() {}
        imContours( const adcontrols::ContoursMethod& t ) : adcontrols::ContoursMethod( t ) {}
    };

    template< typename T, typename ... Algos >
    class ADVISIONSHARED_EXPORT imfilter {
        size_t size_;
        std::tuple< Algos... > algos_;

    public:
        imfilter() : size_( 0 ) {
        }

        imfilter( Algos... algos )
            : size_( sizeof...(Algos) )
            , algos_( std::forward< Algos >( algos )... ) {
        }
        
        ~imfilter() {}

        template< typename R > T operator()( const R&, double scaleFactor = 1.0 ) const;
    };

    // Ex: 
    // qImage = imfilter< QImage >()( boost::ublas::matrix<duble>& );

    // convert ublas::matrix to 8bit gray scale
    template<>
    template<>
    //QImage imfilter< QImage, imGrayScale >::operator()< matrix< double > >( const matrix< double >& ) const;
    QImage imfilter< QImage, imGrayScale >::operator()<>( const boost::numeric::ublas::matrix< double >&, double ) const;

    // convert ublas::matrix apply colormap
    template<>
    template<>
    QImage imfilter< QImage, imColorMap >::operator()<>( const boost::numeric::ublas::matrix< double >&, double ) const;

    // convert ublas::matrix apply colormap
    template<>
    template<>
    QImage imfilter< QImage, imColorMap, imBlur >::operator()<>( const boost::numeric::ublas::matrix< double >&, double ) const;

    //
    template<>
    template<>
    QImage imfilter< QImage, imContours >::operator()<>( const boost::numeric::ublas::matrix< double >&, double ) const;
}
