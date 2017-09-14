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

#include <driver_types.h> // cudaStream_t
#include <thrust/device_vector.h>

namespace cuda {

    template< typename T >
    class device_ptr {
        T * p_;
        size_t rows_, cols_, channels_;
        device_ptr( const device_ptr& ) = delete;
        device_ptr& operator = ( const device_ptr& ) = delete;

    public:
        device_ptr( device_ptr&& t ) : p_( t.p_ ), rows_( t.rows_ ), cols_( t.cols_ ), channels_( t.channels_ ) {
            t.p_ = 0; // don't delete
        }
        
        device_ptr() : p_( 0 ), rows_( 0 ), cols_( 0 ), channels_( 0 ) {
        }
        
        device_ptr( T * p, size_t rows, size_t cols, size_t channels ) : p_( p )
                                                                       , rows_( rows )
                                                                       , cols_( cols )
                                                                       , channels_( channels ) {
        }
        
        ~device_ptr() {
            cudaFree( p_ );
        }

        size_t rows() const { return rows_; }
        size_t cols() const { return cols_; }
        size_t channels() const { return channels_; }

        T * get() { return p_; }

        const T * get() const { return p_; }
    };

}
