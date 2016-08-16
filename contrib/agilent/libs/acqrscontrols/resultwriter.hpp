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

#pragma once

#include <adicontroller/task.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace acqrscontrols {

    template< typename result_type, typename access_type >
    class ResultWriter {
    public:
        ResultWriter( const boost::uuids::uuid& uuid ) : uuid_( uuid )
            {}

        ~ResultWriter()
            {}

        ResultWriter& operator << ( const result_type result ) {
            std::lock_guard< std::mutex > lock( mutex_ );
            cache_.emplace_back( result );
            return *this;
        }

        void commitData() {
            auto accessor = std::make_shared< access_type >();
            do {
                std::lock_guard< std::mutex > lock( mutex_ );
                if ( cache_.size() >= 3 ) {
                    auto tail = std::next( cache_.begin(), cache_.size() - 2 );
                    std::move( cache_.begin(), tail, std::back_inserter( accessor->list ) );
                    cache_.erase( cache_.begin(), tail );
                }
            } while ( 0 );

            if ( !accessor->list.empty() ) {
                auto dataWriter = std::make_shared< adicontroller::SignalObserver::DataWriter >( accessor );
                adicontroller::task::instance()->handle_write( uuid_, dataWriter );
            }
        }

    private:
        std::mutex mutex_;
        std::vector< result_type > cache_;
        const boost::uuids::uuid uuid_;
    };

}

