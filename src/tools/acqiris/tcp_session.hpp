/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <boost/asio.hpp>
#include <boost/variant.hpp>
#include <atomic>
#include <chrono>
#include <memory>

class tcp_session : public std::enable_shared_from_this< tcp_session > {
public:
    session( boost::asio::ip::tcp::socket socket );
    ~session();
    void start();
    void write();
    void acq_start( bool );
    void set_delay_count( uint32_t delay_count );

    static void initialize();
    
private:
    void do_read();
    void do_write();

    boost::asio::ip::tcp::socket socket_;
    enum { max_length = 512 * 512 * sizeof(uint16_t) + 1024 };
    std::array< char, max_length > rdata_;
    std::atomic<bool> stop_requested_;
};

