/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#ifndef TASK_HPP
#define TASK_HPP

#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <boost/asio.hpp>
#include <boost/any.hpp>

namespace batchproc {

    class task : boost::noncopyable {
        task();
        static task * instance_;
        static std::mutex mutex_;
        
    public:
        static task * instance();
        static bool shutdown();

        template<class T> void post( T& doit ) {
            io_service_.post( [&]{ doit(); } );
            std::lock_guard< std::mutex > lock( mutex_ );
            processes_.push_back( doit.shared_from_this() );
        }
        template<class T> void remove( T& me ) {
            auto ptr = me.shared_from_this();
            std::lock_guard< std::mutex > lock( mutex_ );
            auto it = std::find_if( processes_.begin(), processes_.end(), [=]( boost::any& a ){
                    return ((a.type() == typeid( std::shared_ptr<T> ))
                            && (boost::any_cast<std::shared_ptr<T> >(a) == ptr ) );
                });
            if ( it != processes_.end() )
                processes_.erase( it );
        }

    private:
        std::vector< std::thread > threads_;
        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        std::vector< boost::any > processes_;
        void open();
    };

}

#endif // TASK_HPP
