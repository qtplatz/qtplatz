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

#include <adinterface/receiverS.h>
#include <functional>

namespace acquire {

    class receiver_i : public POA_Receiver {
    protected:
        void message(::Receiver::eINSTEVENT msg, ::CORBA::ULong value ) override {
            if ( message_ )
                message_( msg, value );
        }
        void log( const ::EventLog::LogMessage & log ) override {
            if ( log_ )
                log_( log );
        }
        void shutdown(void) override {
            if ( shutdown_ )
                shutdown_();
        }
        void debug_print(::CORBA::Long priority, ::CORBA::Long category, const char * text) override {
            if ( debug_print_ )
                debug_print_( priority, category, std::string( text ) );
        }
    public:
        void assign_message( std::function< void( ::Receiver::eINSTEVENT, uint32_t ) > f ) {
            message_ = f;
        }
        void assign_log( std::function< void( const ::EventLog::LogMessage& ) > f ) {
            log_ = f;
        }
        void assign_shutdown( std::function< void() > f ) {
            shutdown_ = f;
        }
        void assign_debug_print( std::function< void( int32_t, int32_t, std::string ) > f ) {
            debug_print_ = f;
        }
    private:
        std::function< void( ::Receiver::eINSTEVENT, uint32_t ) > message_;
        std::function< void( const ::EventLog::LogMessage& ) > log_;
        std::function< void() > shutdown_;
        std::function< void( int32_t, int32_t, std::string ) > debug_print_;
    };

}

