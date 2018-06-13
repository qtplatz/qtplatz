/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "softaveraged_waveform_accessor.hpp"
#include <acqrscontrols/u5303a/waveform.hpp>
#include <adportable/debug.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

using namespace acqrscontrols;

softaveraged_waveform_accessor::softaveraged_waveform_accessor()
{
}
        
size_t
softaveraged_waveform_accessor::ndata() const
{
    return avgd.size();
}     // number of data in the buffer

void
softaveraged_waveform_accessor::rewind() 
{ 
    it_ = avgd.begin();
}
        
bool
softaveraged_waveform_accessor::next()
{
    return ++it_ != avgd.end();
}

uint64_t
softaveraged_waveform_accessor::elapsed_time() const
{
    return uint64_t( (*it_)->meta_.initialXTimeSeconds * 1.0e9 );
}
        
uint64_t
softaveraged_waveform_accessor::epoch_time() const
{
    return (*it_)->timeSinceEpoch_;
}
        
uint64_t
softaveraged_waveform_accessor::pos() const
{
    return (*it_)->serialnumber_;
}
        
uint32_t
softaveraged_waveform_accessor::fcn() const
{
    return ( *it_ )->method_.protocolIndex();
}
        
uint32_t
softaveraged_waveform_accessor::events() const
{
    return (*it_)->wellKnownEvents_;
}
        
size_t
softaveraged_waveform_accessor::xdata( std::string& ar ) const
{
    return (*it_)->serialize_xdata( ar );
 }
        
size_t
softaveraged_waveform_accessor::xmeta( std::string& ar ) const
{
    return (*it_)->serialize_xmeta( ar );    
}
