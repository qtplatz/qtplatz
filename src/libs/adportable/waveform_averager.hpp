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
#include <functional>
#include <stdexcept>

namespace adportable {

    template< typename lvalue_type >
    class waveform_averager {
        size_t size_;
        size_t actualAverages_;
        std::unique_ptr< lvalue_type [] > data_;

    public:
        ~waveform_averager() {
        }
        
        waveform_averager() : size_( 0 ), actualAverages_( 0 ) {
        }

        waveform_averager( size_t size ) : size_( size )
                                         , actualAverages_( 0 )
                                         , data_( new lvalue_type [ size_ ] ) {
        }
        
        template< typename waveform_type >
        waveform_averager( const waveform_type& t ) : size_( t.size() )
                                                    , actualAverages_( 1 )
                                                    , data_( new lvalue_type[ size_ ] ) {
            std::copy( t.begin(), t.end(), data_.get() );            
        }
            
        template< typename waveform_type >
        waveform_averager& operator += ( const waveform_type& w ) {
            if ( size_ == 0 ) {
                return *new (this) waveform_averager< lvalue_type >;
            } else if ( size_ == w.size() ) {
                std::transform( w.begin(), w.end(), data_.get(), data_.get(), std::plus<int>() );
                ++actualAverages_;
            } else
                throw std::out_of_range("waveform length mismatch");
            return *this;
        }

        const size_t actualAverages() const { return actualAverages_; }

        const size_t size() const { return size_; }

        std::unique_ptr< lvalue_type [] >& data() { return data_; }
    };
    

}

