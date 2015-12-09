/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "constants.hpp"
#include "adicontroller_global.hpp"
#include <boost/any.hpp>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <compiler/pragma_warning.hpp>

namespace boost { namespace uuids { struct uuid; } }

namespace adicontroller {

    namespace SignalObserver {

        class DataWriter;
        class DataReadBuffer;

#if defined _MSC_VER
        ADICONTROLLERSHARED_TEMPLATE_EXPORT template class ADICONTROLLERSHARED_EXPORT std::weak_ptr < DataWriter > ;
#endif

        class ADICONTROLLERSHARED_EXPORT DataWriter : public std::enable_shared_from_this< DataWriter > {

            DataWriter( const DataWriter& ) = delete;
            void operator = ( const DataWriter& ) = delete;
            
        public:
            typedef bool (serializer)( const boost::any&&, std::string& xdata, std::string& xmeta );
            
            virtual ~DataWriter();
            DataWriter();
            DataWriter( const DataReadBuffer& );
            DataWriter( boost::any&&, uint64_t elapsed_time, uint64_t epock_time, uint64_t pos, uint32_t fcn, uint32_t ndata, uint32_t events );
            
            uint64_t elapsed_time() const;
            uint64_t epoch_time() const;                        
            uint64_t pos() const;       // data address (sequencial number for first data in this frame)
            uint32_t fcn() const;       // function number for spectrum
            uint32_t ndata() const;     // number of data in the buffer
            uint32_t events() const;    // well known events

            const boost::any& data() const;
            void setData( boost::any&& );
            void setSerializer( std::function< serializer > );

            std::function< serializer >& Serializer();

            // bool serialize( const boost::any&, std::string& xdata, std::string& xmeta );

        private:
            std::function< serializer > serializer_;
            
            uint64_t elapsed_time_;  // ns
            uint64_t epoch_time_;    // ns
            uint64_t pos_;           // data address (sequencial number for first data in this frame)
            uint32_t fcn_;           // function number for spectrum
            uint32_t ndata_;         // number of data in the buffer (for trace, spectrum should be always 1)
            uint32_t events_;        // well known events
            pragma_msvc_warning_push_disable_4251
            boost::any any_;
            pragma_msvc_warning_pop
        };
        
    }
}
