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

#include "pkd_result_accessor.hpp"
#include "waveform.hpp"
#include <adportable/debug.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/serialization/vector.hpp>

using namespace aqmd3controls;

pkd_result_accessor::pkd_result_accessor()
{
}


size_t
pkd_result_accessor::ndata() const
{
    return list.size();
}     // number of data in the buffer

void
pkd_result_accessor::rewind()
{
    it_ = list.begin();
}

bool
pkd_result_accessor::next()  {
    return ++it_ != list.end();
}

uint64_t
pkd_result_accessor::elapsed_time() const
{
    return (*it_)->data()->timepoint(); // workaround -- (*it_)->data()->xmeta().elapsed_time_;
}

uint64_t
pkd_result_accessor::epoch_time() const
{
    return (*it_)->data()->epoch_time();
}

uint64_t
pkd_result_accessor::pos() const
{
    return (*it_)->data()->serialnumber(); //serialnumber_;
}

uint32_t
pkd_result_accessor::fcn() const
{
    // auto idx = ( *it_ )->data()->method_.protocolIndex();
    return 0; //idx;
}

uint32_t
pkd_result_accessor::events() const
{
    return (*it_)->data()->well_known_events(); //wellKnownEvents_;
}

size_t
pkd_result_accessor::xdata( std::string& ar ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( ar );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
#if 0
    //std::vector< std::shared_ptr< const aqmd3controls::pkd_result > >::iterator it_;
    //std::vector< const adportable::counting::threshold_index >& indices = (*it_);
    auto& indices = ( *it_ )->indices2();
    try {
        portable_binary_oarchive a( device );
        a << indices;
    } catch ( std::exception& ex ) {
        ADDEBUG() << "exception : " << ex.what();
    }
#endif
    ADDEBUG() << "serialize_xdata todo";
    return ar.size();
}

size_t
pkd_result_accessor::xmeta( std::string& ar ) const
{
    ADDEBUG() << "serialize_xmeta todo";
    return 0; // ( *it_ )->data()->serialize_xmeta( ar );
}
