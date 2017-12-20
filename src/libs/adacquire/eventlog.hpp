/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#pragma once

#include "adacquire_global.hpp"
#include <boost/variant.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace boost {
    namespace serialization { class access; }
}

namespace adacquire {
    
    namespace EventLog {

        template< typename T > class LogMessage_archive;
    
        typedef boost::variant< int32_t, uint32_t, int64_t, uint64_t, double, std::string > LogMessageArgType;

        enum eMSGPRIORITY {
            pri_DEBUG
            , pri_INFO     // informational
            , pri_WARNING  // user correctable, correction not required
            , pri_ERROR    // user correctable error
            , pri_EXCEPT   // exception, can be recoverd. Mostly a bug
            , pri_PANIC // non-recoverable exception
        };
  
        class ADACQUIRESHARED_EXPORT LogMessage {
        public:
            LogMessage();
            LogMessage( const LogMessage& );            
            LogMessage( const std::string& format
                        , const std::string& msgId
                        , const std::string& srcId
                        , uint32_t pri = pri_INFO, uint64_t epoch_time = 0 );

            uint32_t logId() const;
            uint32_t priority() const;
            uint64_t timeSinceEpock() const;
            const std::string& msgId() const;
            const std::string& srcId() const;
            const std::string& format() const;
            const std::vector< LogMessageArgType >& args() const;
            
            void setLogId( uint32_t );
            void setPriority( uint32_t );
            void setTimeSinceEpock( uint64_t ); // nanoseconds
            void setMsgId( const std::string& );
            void setSrcId( const std::string& );
            void setFormat( const std::string& );
            void operator << ( LogMessageArgType && );

            bool xml_restore( std::wistream&& );
            static bool xml_archive( std::wostream& os, const LogMessage& );
            static bool xml_restore( LogMessage&, std::wistream&& );

        private:
            uint32_t  logId_;       // unique number for each log message, such as sequencial number
            uint32_t  priority_;
            uint64_t  epoch_time_;  // time since epoch, nanoseconds
            std::string msgId_;     // message type id, e.g. 'error code'
            std::string srcId_;     // source device id
            std::string format_;
            std::vector< LogMessageArgType > args_;

            friend class LogMessage_archive< LogMessage >;
            friend class LogMessage_archive< const LogMessage >;

            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, const unsigned int );
        };

    } // namespace EventLog
}

BOOST_CLASS_VERSION( adacquire::EventLog::LogMessage, 0 )
