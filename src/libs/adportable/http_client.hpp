// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
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

namespace adportable {

    class http_client {
        
        http_client( const http_client& ) = delete;
        http_client& operator = ( const http_client& ) = delete;

    public:
        ~http_client();        
        http_client( boost::asio::io_service&, const std::string& server );
        
        bool get( const std::string& path, std::ostream& response, unsigned int& status_code );
        bool post( const std::string& path, std::ostream& response, unsigned int& status_code
                   , const char * content_type = "application/xml" );

    private:
        bool sync_write( boost::asio::streambuf& req
                         , unsigned int& status_code
                         , std::string& http_version
                         , std::ostream& result );
            
        boost::asio::io_service& io_service_;
        std::string server_;
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator_;
    };
    
}

