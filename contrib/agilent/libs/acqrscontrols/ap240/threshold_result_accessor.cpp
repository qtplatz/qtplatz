/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "threshold_result_accessor.hpp"
#include <acqrscontrols/ap240/waveform.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adportable_serializer/portable_binary_oarchive.hpp>
#include <adportable_serializer/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/vector.hpp>

using namespace acqrscontrols::ap240;


threshold_result_accessor::threshold_result_accessor()
{
}

size_t
threshold_result_accessor::ndata() const
{
    return list.size();
}

void
threshold_result_accessor::rewind()
{
    it_ = list.begin();
}

bool
threshold_result_accessor::next()
{
    return ++it_ != list.end();
}

uint64_t
threshold_result_accessor::elapsed_time() const
{
    return uint64_t( (*it_)->data()->meta_.initialXTimeSeconds * 1.0e9 );
}

uint64_t
threshold_result_accessor::epoch_time() const
{
    return (*it_)->data()->timeSinceEpoch_;
}

uint64_t
threshold_result_accessor::pos() const
{
    return (*it_)->data()->serialnumber_;
}

uint32_t
threshold_result_accessor::fcn() const
{
   auto idx = ( *it_ )->data()->method_.protocolIndex();
   return idx;
}

uint32_t
threshold_result_accessor::events() const
{
    return (*it_)->data()->wellKnownEvents_;
}

size_t
threshold_result_accessor::xdata( std::string& ar ) const
{
    std::ostringstream o;

    auto& indices = ( *it_ )->indices();
    try {
        portable_binary_oarchive a( o );
        a << indices;
        adportable::bzip2::compress( ar, o.str().data(), o.str().size() );
    } catch ( std::exception& ex ) {
        ADDEBUG() << "exception : " << ex.what();
    }
	return ar.size();
}

size_t
threshold_result_accessor::xmeta( std::string& ar ) const
{
    return ( *it_ )->data()->serialize_xmeta( ar );
}
