// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "adcontrols_global.h"
#include "metric/prefix.hpp"
#include <boost/any.hpp>
#include <boost/variant.hpp>
#include <string>
#include <memory>
#include <vector>

namespace adcontrols {

#if defined _MSC_VER
    template< typename value_type > class waveform;
    ADCONTROLSSHARED_TEMPLATE_EXPORT template class ADCONTROLSSHARED_EXPORT std::weak_ptr < waveform<int32_t> > ;
#endif
    
    template< typename value_type = int32_t > // double | int32 only supported
    class ADCONTROLSSHARED_EXPORT waveform : std::enable_shared_from_this < waveform<value_type> > {
        waveform();
        waveform( const waveform& ) = delete;
        waveform& operator = ( const waveform& ) = delete;
    public:
        ~waveform();

        static std::shared_ptr< waveform< value_type > > make_this();

        typedef typename std::vector< value_type >::iterator iterator;
        typedef typename std::vector< value_type >::const_iterator const_iterator;

        iterator begin()             { return data_.begin(); }
        iterator end()               { return data_.end(); }
        const_iterator begin() const { return begin(); }
        const_iterator end() const   { return end(); }

        void reseize() { data_.resize(); }

        size_t size() const { return data_.size(); }
        int operator [] ( size_t idx ) const { return data_[ idx ] };

        int64_t serialnumber() const { return serialnumber_ } ; // a.k.a. trigger number
        void setSerialnumber( int64_t value ) { serialnumber_ = value; } 
        
        uint32_t wellKnownEvents() const { return wellKnownEvents_; }
        void setWellKnownEvents( uint32_t value ) { wellKnownEvents_ = value; }
        
        double timeSinceEpoch() const { return timeSinceEpoch_; }
        void setTimeSinceEpock( double value ) { timeSinceEpoch_ = value; }
        
        double timeSinceInjection() const { return timeSinceInjection_; }
        void setTimeSinceInjection( double value ) { timeSinceInjection_ = value; }

    private:
        class impl;
        pragma_msvc_warning_push_disable_4251
        std::unique_ptr< impl > impl_;
        std::vector< value_type > data_;
        std::string device_metadata_;
        uint64_t serialnumber_;
        uint32_t wellKnownEvents_;
        double timeSinceEpoch_;
        double timeSinceInjection_;
        pragma_msvc_warning_pop
    };

    // template<> ADCONTROLSSHARED_EXPORT const int16_t * waveform::begin() const;
    // template<> ADCONTROLSSHARED_EXPORT const int16_t * waveform::end() const;
    // template<> ADCONTROLSSHARED_EXPORT const int32_t * waveform::begin() const;
    // template<> ADCONTROLSSHARED_EXPORT const int32_t * waveform::end() const;
    // template<> ADCONTROLSSHARED_EXPORT int16_t * waveform::data();
    // template<> ADCONTROLSSHARED_EXPORT int32_t * waveform::data();
}
