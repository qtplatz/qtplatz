/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "eventlog.hpp"
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <atomic>
#include <chrono>

namespace adicontroller {
    namespace EventLog {
        static std::atomic< uint32_t > __logid;

        template< typename T = LogMessage >
        class LogMessage_archive {
        public:
            template< class Archive >
            void serialize( Archive& ar, T& _, const unsigned int version ) {
                ar & BOOST_SERIALIZATION_NVP( _.logId_ );
                ar & BOOST_SERIALIZATION_NVP( _.priority_ );
                ar & BOOST_SERIALIZATION_NVP( _.epoch_time_ );
                ar & BOOST_SERIALIZATION_NVP( _.msgId_ );
                ar & BOOST_SERIALIZATION_NVP( _.srcId_ );
                ar & BOOST_SERIALIZATION_NVP( _.format_ );
                ar & BOOST_SERIALIZATION_NVP( _.args_ );
            }
        };

        ///////// XML archive ////////
        template<> void
        LogMessage::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
        {
            LogMessage_archive<>().serialize( ar, *this, version );
        }
        
        template<> void
        LogMessage::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
        {
            LogMessage_archive<>().serialize( ar, *this, version );        
        }
        
    }
}

using namespace adicontroller;
using namespace adicontroller::EventLog;

LogMessage::LogMessage() : logId_( __logid++ )
                         , priority_( pri_DEBUG )
                         , epoch_time_( 0 )
{
}

LogMessage::LogMessage( const LogMessage& t ) : logId_( t.logId_ )
                                              , priority_( t.priority_ )
                                              , epoch_time_( t.epoch_time_ )
                                              , msgId_( t.msgId_ )
                                              , srcId_( t.srcId_ )
                                              , format_( t.format_ )
                                              , args_( t.args_ )
{
}

LogMessage::LogMessage( const std::string& format
                        , const std::string& msgId
                        , const std::string& srcId
                        , uint32_t pri
                        , uint64_t epoch_time ) : format_( format )
                                                , msgId_( msgId )
                                                , srcId_( srcId )
                                                , logId_( __logid++ )
                                                , priority_( pri )
                                                , epoch_time_( epoch_time )
{
    if ( epoch_time_ == 0 )
        epoch_time_ = std::chrono::duration_cast< std::chrono::nanoseconds >( std::chrono::system_clock::now().time_since_epoch() ).count();
}

uint32_t
LogMessage::logId() const
{
    return logId_;
}

uint32_t
LogMessage::priority() const
{
    return priority_;
}

uint64_t
LogMessage::timeSinceEpock() const
{
    return epoch_time_;
}

const std::string&
LogMessage::msgId() const
{
    return msgId_;
}

const std::string&
LogMessage::srcId() const
{
    return srcId_;
}

const std::string&
LogMessage::format() const
{
    return format_;
}

const std::vector< LogMessageArgType >&
LogMessage::args() const
{
    return args_;
}

void
LogMessage::setLogId( uint32_t v )
{
    logId_ = v;
}

void
LogMessage::setPriority( uint32_t v )
{
    priority_ = v;
}

void
LogMessage::setTimeSinceEpock( uint64_t v )
{
    epoch_time_ = v;
}

// nanoseconds
void
LogMessage::setMsgId( const std::string& v )
{
    msgId_ = v;
}

void
LogMessage::setSrcId( const std::string& v )
{
    srcId_ = v;
}

void
LogMessage::setFormat( const std::string& v )
{
    format_ = v;
}

void
LogMessage::operator << ( LogMessageArgType&& a )
{
    args_.emplace_back( a );
}

bool
LogMessage::xml_archive( std::wostream& os, const LogMessage& log )
{
    boost::archive::xml_woarchive ar( os );
    try {
        ar & boost::serialization::make_nvp( "LogMessage", log );
        return true;
    } catch ( ... ) {
        return false;
    }
}

bool
LogMessage::xml_restore( LogMessage& log, std::wistream&& is )
{
    boost::archive::xml_wiarchive ar( is );
    try {
        ar & boost::serialization::make_nvp( "LogMessage", log );
        return true;
    } catch ( ... ) {
    }
    return false;
}

bool 
LogMessage::xml_restore( std::wistream&& is )
{
    xml_restore( *this, std::move( is ) );
}

