/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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
#include <adacquire/signalobserver.hpp>
#include <boost/uuid/uuid.hpp>
#include <deque>

namespace socfpga {

    namespace dgmod {
        namespace so = adacquire::SignalObserver;

        struct advalue;

        class TraceObserver : public so::Observer { // inherit from std::enable_shared_from_this<Observer>
            TraceObserver( const TraceObserver& ) = delete;
        public:
            TraceObserver();
            virtual ~TraceObserver();

            constexpr static const boost::uuids::uuid __objid__ = trace_observer;        // dd141c3a-7b57-454c-9515-23242c0345a7
            constexpr static const char * __objtext__           = trace_observer_name;   // "1.httpd-dg.ms-cheminfo.com";
            constexpr static const char * __data_interpreter__  = trace_datainterpreter; // 3c552747-5ef4-41ff-9074-aa8585cb1765

            // Observer methods
            const boost::uuids::uuid& objid() const override { return __objid__; }
            const char * objtext() const override { return __objtext__; }

            uint64_t uptime() const override;
            void uptime_range( uint64_t& oldest, uint64_t& newest ) const override;

            std::shared_ptr< so::DataReadBuffer > readData( uint32_t pos ) override;

            const char * dataInterpreterClsid() const override { return __data_interpreter__; }

            int32_t posFromTime( uint64_t usec ) const override;
            bool prepareStorage( adacquire::SampleProcessor& ) const override { return false; }
            bool closingStorage( adacquire::SampleProcessor& ) const override { return false; }

            // TraceObserver (local)
            uint32_t emplace_back( std::vector< advalue >&& );

        private:
            std::vector< std::shared_ptr< so::DataReadBuffer > > que_;
            uint32_t rx_pos_;
        };
    }
}
