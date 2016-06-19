/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "receiverimpl.hpp"

class QString;

using namespace adextension;

ReceiverImpl::~ReceiverImpl()
{
}

ReceiverImpl::ReceiverImpl( iController * p ) : controller_( p->pThis() )
{
}

            
void
ReceiverImpl::message( eINSTEVENT msg, uint32_t value )
{
    if ( auto p = controller_.lock() )
        emit p->message( p.get(), unsigned( msg ), unsigned( value ) );
}
            
void
ReceiverImpl::log( const adicontroller::EventLog::LogMessage& log )
{
}

void
ReceiverImpl::shutdown()
{
}
            
void
ReceiverImpl::debug_print( uint32_t priority, uint32_t category, const std::string& text )
{
}

void
ReceiverImpl::notify_error( const boost::system::error_code& ec , const std::string& file , int line )
{
    if ( auto p = controller_.lock() )
        emit p->notifyError( p.get(), QString::fromStdString( ec.message() ), QString::fromStdString( file ), line );
}

void
ReceiverImpl::notify_error( const std::string& what , const std::string& file , int line )
{
    if ( auto p = controller_.lock() )
        emit p->notifyError( p.get(), QString::fromStdString( what ), QString::fromStdString( file ), line );    
}
