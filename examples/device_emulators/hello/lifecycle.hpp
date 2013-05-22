/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#ifndef LIFECYCLE_HPP
#define LIFECYCLE_HPP

#include <boost/asio.hpp>
#include <boost/cstdint.hpp>

enum LifeCycleCommand {
    NOTHING = 0
    , HELO         = 0xffff0720
    , CONN_SYN     = 0x20100721
    , CONN_SYN_ACK = 0x20100722
    , CLOSE        = 0x20100723
    , CLOSE_ACK    = 0x20100724
    , DATA         = 0x20100725
    , DATA_ACK     = 0x20100726
};

struct LifeCycleFrame {
    boost::uint16_t endian_mark;    // 0xfffe
    boost::uint16_t proto_version;  // 0x0001
    boost::uint16_t ctrl;           // 0
    boost::uint16_t hoffset;        // 8
    boost::uint32_t command;        // CONN_SYN etc.
    LifeCycleFrame( LifeCycleCommand cmd = NOTHING ) : endian_mark( 0xfffe )
                                                     , proto_version( 0x0001 )
                                                     , ctrl( 0 )
                                                     , hoffset( 8 )
                                                     , command( cmd ) {
    }
};

class lifecycle {
public:
    lifecycle();
    virtual bool operator()( const boost::asio::ip::udp::endpoint&, const char *, std::size_t );
    void register_client( lifecycle * );
private:
    lifecycle * forward_;
};

#endif // LIFECYCLE_HPP
