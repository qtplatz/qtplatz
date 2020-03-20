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

#include "singleton.hpp"
#include "dataprocessor.hpp"
#include <boost/filesystem.hpp>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using namespace ws_adprocessor;

singleton::singleton() : idCounter_(1)
{
}

singleton::~singleton()
{
}

singleton *
singleton::instance()
{
    static singleton __instance;
    return &__instance;
}

uint32_t
singleton::generateId()
{
    auto id = idCounter_++;
    return id;
}

int
singleton::set_dataProcessor( std::shared_ptr< ws_adprocessor::dataProcessor > dp )
{
    auto id = generateId();
    dataList_[ id ] = std::make_tuple( dp, std::chrono::system_clock::now() );
    return id;
}

void
singleton::remove_dataProcessor( int id )
{
    dataList_.erase( id );
}

std::shared_ptr< dataProcessor >
singleton::dataProcessor( int id )
{
    auto it = dataList_.find( id );
    if ( it != dataList_.end() ) {
        std::get<1>( it->second ) = std::chrono::system_clock::now(); // update access time
        return std::get<0>( it->second );
    }
    return nullptr;
}
