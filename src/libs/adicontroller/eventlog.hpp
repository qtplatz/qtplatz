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

#pragma once

#include "adicontroller_global.hpp"
#include <cstdint>
#include <string>
#include <vector>

// Logging mechanism for instrument control server.
// This is not intend for Broker that is document server for data processing.
// See broker.idl for details.  T.H.

namespace adicontroller {

    class ADICONTROLLERSHARED_EXPORT EventLog {
    public:

        virtual ~EventLog();
        EventLog();
        
        enum eMSGPRIORITY {
            pri_DEBUG
            , pri_INFO     // informational
            , pri_WARNING  // user correctable, correction not required
            , pri_ERROR    // user correctable error
            , pri_EXCEPT   // exception, can be recoverd. Mostly a bug
            , pri_PANIC // non-recoverable exception
        };
  
        struct LogMessage {
            LogMessage();
            uint32_t logId() const;
            uint32_t priority() const;
            uint64_t timeSinceEpock() const;
            const char * msgId() const;
            const char * srcId() const;
            const char * format() const;
            const std::vector< std::string >& args() const;
            
            void setLogId( uint32_t );
            void setPriority( uint32_t );
            void setTimeSinceEpock( uint64_t ); // nanoseconds
            void setMsgId( const std::string& );
            void setSrcId( const std::string& );
            void setFormat( const std::string& );
            void setArgs( const std::vector< std::string >& );

        private:
            uint32_t logId_;       // unique number for each log message, such as sequencial number
            uint32_t priority_;
            uint64_t  tv_;         // time since epoc, microseconds
# if defined _MSC_VER
#  pragma warning(push)
#  pragma warning(disable:4251)
# endif
            std::string msgId_;    // such as 'error code' for end user
            std::string srcId_;    // source device id
            std::string format_;
            std::vector< std::string > args_;
# if defined _MSC_VER
#  pragma warning(pop)
# endif
        };
        
    };
    
}
