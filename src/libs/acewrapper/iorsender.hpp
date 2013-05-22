/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#ifndef IORSENDER_HPP
#define IORSENDER_HPP

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <string>
#include <map>

template<class T, class M> class ACE_Singleton;
class ACE_Recursive_Thread_Mutex;

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

	boost::asio::io_service io_service_;
	boost::asio::ip::udp::socket socket_;
	boost::asio::ip::udp::endpoint sender_endpoint_;
	boost::array< char, 1024u > recv_buffer_;
	std::vector< char > send_buffer_;
	std::map< std::string, std::string > iorvec_;
	std::map< std::string, std::string >::iterator nextIor_;
        boost::mutex mutex_;
        boost::thread * thread_;
    };

}

#endif // IORSENDER_HPP
