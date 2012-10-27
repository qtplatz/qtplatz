/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef IORSENDER_HPP
#define IORSENDER_HPP

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <ace/Singleton.h>
#include <ace/Recursive_Thread_Mutex.h>
#include <vector>
#include <string>
#include <map>

namespace acewrapper {

    class iorSender : boost::noncopyable {
	iorSender();
    public:
	static iorSender * instance();
	bool open( unsigned short port = 0 );
	void close();

	void register_lookup( const std::string& ior, const std::string& ident );
	void unregister_lookup( const std::string& ident );

	bool spawn();

        friend class ACE_Singleton< iorSender, ACE_Recursive_Thread_Mutex >;
    private:
	static void * thread_entry( void * );

	void start_receive();
	void handle_timeout( const boost::system::error_code& );
	void handle_receive( const boost::system::error_code&, std::size_t );
	void handle_sendto( const boost::system::error_code& );

	bool thread_running_;
	ACE_thread_t t_handle_;
	boost::asio::io_service io_service_;
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	boost::array< char, 1024u > recv_buffer_;
	std::vector< char > send_buffer_;
	std::map< std::string, std::string > iorvec_;
	std::map< std::string, std::string >::iterator nextIor_;
	ACE_Recursive_Thread_Mutex mutex_;
    };

}

#endif // IORSENDER_HPP
