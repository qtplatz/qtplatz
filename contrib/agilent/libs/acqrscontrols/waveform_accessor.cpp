/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "waveform_accessor.hpp"
#include <acqrscontrols/u5303a/waveform.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <adportable/debug.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/vector.hpp>

namespace acqrscontrols {
    //////////// AP240 
    template<>
    uint64_t
    waveform_accessor_< acqrscontrols::ap240::waveform >::elapsed_time() const
    {
        return uint64_t( (*it_)->meta_.initialXTimeSeconds * 1.0e9 );
    }

    template<>
    uint64_t
    waveform_accessor_< acqrscontrols::ap240::waveform >::epoch_time() const
    {
        return (*it_)->timeSinceEpoch_;
    }

    template<>
    uint64_t
    waveform_accessor_< acqrscontrols::ap240::waveform >::pos() const
    {
        return (*it_)->serialnumber_;
    }

    template<>
    uint32_t
    waveform_accessor_< acqrscontrols::ap240::waveform >::fcn() const
    {
        auto idx = ( *it_ )->method_.protocolIndex();
        return idx;
    }

    template<>
    uint32_t
    waveform_accessor_< acqrscontrols::ap240::waveform >::events() const
    {
        return (*it_)->wellKnownEvents_;
    }

    template<>
    size_t
    waveform_accessor_< acqrscontrols::ap240::waveform >::xdata( std::string& ar ) const
    {
        std::string o;
        if ( (*it_)->serialize_xdata( o ) )
            adportable::bzip2::compress( ar, o.data(), o.size() );
        return ar.size();
    }

    template<>
    size_t
    waveform_accessor_< acqrscontrols::ap240::waveform >::xmeta( std::string& ar ) const
    {
        return ( *it_ )->serialize_xmeta( ar );
    }

    //////////////////////////////////////////////////
    //////////// U5303A 
    template<>
    uint64_t
    waveform_accessor_< acqrscontrols::u5303a::waveform >::elapsed_time() const
    {
        return uint64_t( (*it_)->meta_.initialXTimeSeconds * 1.0e9 );
    }

    template<>
    uint64_t
    waveform_accessor_< acqrscontrols::u5303a::waveform >::epoch_time() const
    {
        return (*it_)->timeSinceEpoch_;
    }

    template<>
    uint64_t
    waveform_accessor_< acqrscontrols::u5303a::waveform >::pos() const
    {
        return (*it_)->serialnumber_;
    }

    template<>
    uint32_t
    waveform_accessor_< acqrscontrols::u5303a::waveform >::fcn() const
    {
        auto idx = ( *it_ )->method_.protocolIndex();
        return idx;
    }

    template<>
    uint32_t
    waveform_accessor_< acqrscontrols::u5303a::waveform >::events() const
    {
        // ADDEBUG() << "u5303a waveform events: " << (*it_)->wellKnownEvents_;
        return (*it_)->wellKnownEvents_;
    }

    template<>
    size_t
    waveform_accessor_< acqrscontrols::u5303a::waveform >::xdata( std::string& ar ) const
    {
        std::string o;
        if ( (*it_)->serialize_xdata( o ) )
            adportable::bzip2::compress( ar, o.data(), o.size() );
        return ar.size();
    }

    template<>
    size_t
    waveform_accessor_< acqrscontrols::u5303a::waveform >::xmeta( std::string& ar ) const
    {
        return ( *it_ )->serialize_xmeta( ar );
    }
}
