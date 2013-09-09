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

#include <adinterface/brokereventS.h>
#include <functional>

namespace acquire {

    class brokerevent_i : public POA_BrokerEventSink {
    protected:
        void message( const char * message ) override {
            if ( message_ )
                message_( message );
        }
        void portfolio_created( const ::CORBA::WChar * token ) override {
            if ( portfolio_created_ )
                portfolio_created_( token );
        }
        void folium_added( const ::CORBA::WChar * token
                           , const ::CORBA::WChar * path
                           , const ::CORBA::WChar * folderId ) override {
            if ( folium_added_ )
                folium_added_( token, path, folderId );
        }

    public:
        void assign_message( std::function< void( const std::string& ) > f ) {
            message_ = f;
        }
        void assign_portfolio_created( std::function< void( const std::wstring& ) > f ) {
            portfolio_created_ = f;
        }
        void assign_folium_added( std::function< void( const std::wstring&, const std::wstring&, const std::wstring& ) > f ) {
            folium_added_ = f;
        }

    private:
        std::function< void( const std::string& ) > message_;
        std::function< void( const std::wstring& ) > portfolio_created_;
        std::function< void( const std::wstring&, const std::wstring&, const std::wstring& ) > folium_added_;
    };

}

