/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include <QCoreApplication>

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

enum { max_length = 1024 };

class client {
public:
	client(boost::asio::io_service& io_service, boost::asio::ssl::context& context,
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator)	: socket_(io_service, context)	{

			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
			socket_.lowest_layer().async_connect(endpoint,
				boost::bind(&client::handle_connect, this,
				boost::asio::placeholders::error, ++endpoint_iterator));
	}

	void handle_connect(const boost::system::error_code& error,
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator)  {
			if (!error)	{
				socket_.async_handshake(boost::asio::ssl::stream_base::client,
					boost::bind(&client::handle_handshake, this,
					boost::asio::placeholders::error));
			} else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())	{
				socket_.lowest_layer().close();
				boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
				socket_.lowest_layer().async_connect(endpoint,
					boost::bind(&client::handle_connect, this,
					boost::asio::placeholders::error, ++endpoint_iterator));
			} else {
				std::cout << "Connect failed: " << error << "\n";
			}
	}

	void handle_handshake(const boost::system::error_code& error)  {
		if (!error)  {
			std::cout << "Enter message: ";
			std::cin.getline(request_, max_length);
			size_t request_length = strlen(request_);

			boost::asio::async_write(socket_,
				boost::asio::buffer(request_, request_length),
				boost::bind(&client::handle_write, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		} else  {
			std::cout << "Handshake failed: " << error << "\n";
		}
	}

	void handle_write(const boost::system::error_code& error,
		size_t bytes_transferred)  {
			if (!error) {
				boost::asio::async_read(socket_,
					boost::asio::buffer(reply_, bytes_transferred),
					boost::bind(&client::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
			} else {
				std::cout << "Write failed: " << error << "\n";
			}
	}

	void handle_read(const boost::system::error_code& error,  size_t bytes_transferred)	{
		if (!error)    {
			std::cout << "Reply: ";
			std::cout.write(reply_, bytes_transferred);
			std::cout << "\n";
		} else {
			std::cout << "Read failed: " << error << "\n";
		}
	}

private:
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
	char request_[max_length];
	char reply_[max_length];
};

int
main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    
	try	{
		if (argc != 3)	{
			std::cerr << "Usage: client <host> <port>\n";
			return 1;
		}

		boost::asio::io_service io_service;

		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query(argv[1], argv[2]);
		boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

		boost::asio::ssl::context ctx(io_service, boost::asio::ssl::context::sslv23);
		ctx.set_verify_mode(boost::asio::ssl::context::verify_peer);
		ctx.load_verify_file("ca.pem");

		client c(io_service, ctx, iterator);

		io_service.run();
	} catch (std::exception& e)	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
	return 0;
}

