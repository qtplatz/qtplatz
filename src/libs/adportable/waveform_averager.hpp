// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "waveform_wrapper.hpp"

namespace adportable {

    template< typename lvalue_type, typename allocator = std::allocator< lvalue_type > >
    class waveform_averager {
        size_t size_;
        size_t actualAverages_;
        std::unique_ptr< lvalue_type [], allocator > data_;

    public:
        ~waveform_averager() {
        }
        
        waveform_averager() : size_( 0 ), actualAverages_( 0 ) {
        }

        waveform_averager( size_t size ) : size_( size )
                                         , actualAverages_( 0 )
                                         , data_( allocator().alloc( size ) ) {
        }
        
        template< typename rvalue_type, typename waveform_type >
        waveform_averager( const waveform_wrapper< rvalue_type, waveform_type >& t ) : size_( t.size() )
                                                                                     , data_( allocator().alloc( size_ ) ) {
            std::copy( t.begin(), t.end(), data_.get() );            
        }
            
        template< typename rvalue_type, typename waveform_type >
        waveform_averager& operator += ( const waveform_wrapper< rvalue_type, waveform_type >& w ) {
            if ( size_ == 0 ) {
                return *new (this) waveform_averager< lvalue_type, allocator >;
            } else if ( size_ == w.size() ) {
                std::transform( data_.get(), data_.get() + size_, w.begin(), std::plus<lvalue_type>() );
                ++actualAverages_;
            } else
                throw std::out_of_range();
            return *this;
        }
        
    };
    

}

