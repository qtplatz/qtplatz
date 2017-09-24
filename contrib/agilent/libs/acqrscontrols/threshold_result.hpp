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

#include "acqrscontrols_global.hpp"
#include <adportable/counting/threshold_index.hpp>
#include <adportable/counting/counting_result.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/access.hpp>
#include <memory>
#include <vector>
#include <cstdint>
#include <ostream>
#include <compiler/pragma_warning.hpp>

// this class was derived from both ap240::threshold_result and u5303a::threshold_result
// for AP240 support on infitof2
// 2017-SEP-18

namespace acqrscontrols {

    class threshold_result : public adportable::counting::counting_result {
    public:
        
# if defined _MSC_VER && _MSC_VER <= 1800
        static const uint32_t npos = (-1);
# else
        static constexpr uint32_t npos = ( -1 );
# endif
        
        threshold_result();
        threshold_result( const threshold_result& t );

        std::vector< double >& processed();
        const std::vector< double >& processed() const;
        const std::pair<uint32_t, uint32_t >& findRange() const;
        uint32_t foundIndex() const;
        void setFoundAction( uint32_t index, const std::pair< uint32_t, uint32_t >& );
        void setFindUp( bool value );
        bool findUp() const;
        
        bool deserialize( const int8_t * data, size_t dsize );
        
        static void write3( std::ostream&, const threshold_result& );

        std::vector< uint32_t >& indecies();
        const std::vector< uint32_t >& indecies() const;
        
    private:
        std::vector< uint32_t > indecies_;
        std::vector< double > processed_;
        std::pair< uint32_t, uint32_t > findRange_;
        uint32_t foundIndex_;
        bool findUp_;
    };

    ////////////////////////////////////////////////////////////////////////////////////////

    template< typename waveform_type >
    class ACQRSCONTROLSSHARED_EXPORT threshold_result_ : public threshold_result {
    public:
        threshold_result_() {}
        
        threshold_result_( std::shared_ptr< const waveform_type > d )
            : data_( d ) {
        }
        
        threshold_result_( const threshold_result_& t ) : threshold_result( t )
            , adportable::counting::counting_result( t )
            , data_( t.data_ ) {
        }
        
        std::shared_ptr< const waveform_type >& data()      { return data_;  }
        std::shared_ptr< const waveform_type > data() const { return data_; }

        bool deserialize( const int8_t * xdata, size_t dsize, const int8_t * xmeta, size_t msize ) {
            //
            // see threshold_result_accessor.cpp in infitof/plugins/infitof2
            //
            auto data = std::make_shared< waveform_type >();
            data->deserialize_xmeta( reinterpret_cast<const char *>( xmeta ), msize );
            data_ = std::move( data );
            return threshold_result::deserialize( xdata, dsize );
        }

    private:
        std::shared_ptr< const waveform_type > data_;
    };
}
