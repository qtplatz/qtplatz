/**************************************************************************
** Copyright (C) 2018-2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this
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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ostream>
#include <map>
#include <memory>
#include <mutex>
#include <tuple>
#include <boost/uuid/uuid.hpp>

namespace boost { namespace filesystem { class path; } };

namespace ws_adprocessor {

    class dataProcessor;

    typedef std::tuple< std::shared_ptr< dataProcessor >, std::chrono::system_clock::time_point > dataListType;
    
    class singleton {
        singleton();
        singleton( const singleton& ) = delete;
        singleton& operator = ( singleton& ) = delete;
    public:
        ~singleton();
        static singleton * instance();

        int set_dataProcessor( std::shared_ptr< dataProcessor > dp );
        void remove_dataProcessor( int );
        std::shared_ptr< dataProcessor > dataProcessor( int );
        uint32_t generateId();

    private:
        std::map< int, dataListType > dataList_;
        std::atomic< uint32_t > idCounter_;
    };

}
