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

#include "threshold_result.hpp"

#include <adportable/counting/threshold_index.hpp>
#include <adportable/counting/counting_result.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <memory>
#include <vector>
#include <cstdint>
#include <ostream>
#include "ap240/threshold_result.hpp"

using namespace acqrscontrols;

threshold_result::threshold_result() : findRange_( 0, 0 )
                                     , foundIndex_( npos )
                                     , findUp_( false )
{
}

threshold_result::threshold_result( const threshold_result& t )
    : adportable::counting::counting_result( t )
    , findRange_( t.findRange_ )
    , foundIndex_( t.foundIndex_ )
    , findUp_( false )
{
}


std::vector< double >&
threshold_result::processed()
{
    return processed_;
}
    
const std::vector< double >&
threshold_result::processed() const
{
    return processed_;            
}

const std::pair<uint32_t, uint32_t >&
threshold_result::findRange() const
{
    return findRange_;
}
    
uint32_t
threshold_result::foundIndex() const
{
    return foundIndex_;
}
    
void
threshold_result::setFoundAction( uint32_t index, const std::pair< uint32_t, uint32_t >& range )
{
    foundIndex_ = index;
    findRange_ = range;
}
    
void
threshold_result::setFindUp( bool value )
{
    findUp_ = value;
}
    
bool
threshold_result::findUp() const
{
    return findUp_;
}

std::vector< uint32_t >&
threshold_result::indecies()
{
    return indecies_;
}

const std::vector< uint32_t >&
threshold_result::indecies() const
{
    return indecies_;    
}

bool
threshold_result::deserialize( const int8_t * xdata, size_t dsize )
{
    // restore indecies
    boost::iostreams::basic_array_source< char > device( reinterpret_cast< const char *>(xdata), dsize );
    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > st( device );
    
    try {
        portable_binary_iarchive ar( st );
        ar >> indecies_;
    } catch ( std::exception& ) {
        return false;
    }
    
    return true;
}

ACQRSCONTROLSSHARED_EXPORT std::ostream & operator<<(std::ostream & os, const ap240_threshold_result &)
{
	return os;
}
