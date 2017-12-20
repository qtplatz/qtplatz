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

#include "timedigital_histogram_accessor.hpp"
#include <adportable/debug.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

using namespace adacquire;

timedigital_histogram_accessor::timedigital_histogram_accessor()
{
}
        
size_t
timedigital_histogram_accessor::ndata() const
{
    return vec.size();
}     // number of data in the buffer

void
timedigital_histogram_accessor::rewind() 
{ 
    it_ = vec.begin();
}
        
bool
timedigital_histogram_accessor::next()
{
    return ++it_ != vec.end();
}

uint64_t
timedigital_histogram_accessor::elapsed_time() const
{
    return uint64_t( (*it_)->initialXTimeSeconds() * 1.0e9 );
}
        
uint64_t
timedigital_histogram_accessor::epoch_time() const
{
    return (*it_)->timeSinceEpoch().first;
}
        
uint64_t
timedigital_histogram_accessor::pos() const
{
    return (*it_)->serialnumber().first;
}
        
uint32_t
timedigital_histogram_accessor::fcn() const
{
    return 0;
}
        
uint32_t
timedigital_histogram_accessor::events() const
{
    return (*it_)->wellKnownEvents();
}
        
size_t
timedigital_histogram_accessor::xdata( std::string& ar ) const
{
    boost::iostreams::back_insert_device< std::string > inserter( ar );
    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
    try {
        portable_binary_oarchive a( device );
        a & *(*it_ );
    } catch ( std::exception& ex ) {
        ADDEBUG() << "exception : " << ex.what();
    }
	return ar.size();
    // return T::archive( device, t );
 }
        
size_t
timedigital_histogram_accessor::xmeta( std::string& ) const
{
    return 0;
}
