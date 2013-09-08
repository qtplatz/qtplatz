// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "brokereventS.h"

namespace adinterface {

    class BrokerEventSink_i : POA_BrokerEventSink {
    protected:
        // implement interface
        void message( CORBA::Char * message ) override { // send message to client
            if ( message_ )
                message_( std::string( message ) );
        }

        void portfolio_created( CORBA::WChar * token ) override {
            if ( portfolio_created_ )
                portfolio_created_( std::wstring( token ) );
        }

        void folium_added( CORBA::WChar * token, CORBA::WChar * path, CORBA::WChar * folderId ) override {
            if ( folium_added_ )
                folium_added_( std::wstring(token), std::wstring(path), std::wstring(folderId) );
        }
    public:
        // assin methods
        void assin_message( std::function< void( std::string ) > f ) {
            message_ = f;
        }
        void assign_portfolio_created( std::function< void( std::wstring ) f > ) {
            portfolio_created_ = f;
        }
        void assign_folium_added( std::function< void( std::wstring, std::wstring, std::wstring ) > f ) {
            f = folium_added_;
        }
    private:
        std::function< void( std::string ) > message_;
        std::function< void( std::wstring ) > portfolio_created_;
        std::function< void( std::wstring, std::wstring, std::wstring ) > folium_added_;
    };
};
