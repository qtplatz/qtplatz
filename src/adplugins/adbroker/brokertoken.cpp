// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#include "brokertoken.hpp"
#include <time.h>

BrokerToken::~BrokerToken()
{
}

BrokerToken::BrokerToken()
{
}

BrokerToken::BrokerToken( const BrokerToken& t ) : connString_(t.connString_)
												 , terminalId_(t.terminalId_)
												 , sessionGuid_(t.sessionGuid_)
												 , ownerId_(t.ownerId_)
												 , ownerName_(t.ownerName_)
												 , connDate_(t.connDate_)
												 , revision_(t.revision_)
{
}

const std::wstring& 
BrokerToken::connDate() const
{
  return connDate_;
}

const std::wstring& 
BrokerToken::connString() const
{
  return connString_;
}

const std::wstring& 
BrokerToken::terminalId() const
{
  return terminalId_;
}

const std::wstring& 
BrokerToken::sessionGuid() const
{
  return sessionGuid_;
}

const std::wstring& 
BrokerToken::ownerId() const
{
  return ownerId_;
}

const std::wstring& 
BrokerToken::ownerName() const
{
  return ownerName_;
}

const std::wstring& 
BrokerToken::revision() const
{
  return revision_;
}

//static 
BrokerToken
BrokerToken::Create( const std::wstring& ownerId
					 , const std::wstring& ownerName
					 , const std::wstring& sessionGuid
					 , const std::wstring& terminalId
					 , const std::wstring& connString
                                         , const std::wstring& revision )
{
  BrokerToken token;
  token.ownerId_     = ownerId;
  token.ownerName_   = ownerName;
  token.sessionGuid_ = sessionGuid;
  token.terminalId_  = terminalId;
  token.connString_  = connString;
  token.revision_    = revision;
  
  time_t tm;
  time(&tm);
  // _date_t today(tm);
  // token.connDate_ = _wstr_t::towcs( std::string( today ) );

  return token;
}
  
//static
BrokerToken
BrokerToken::fromXml( const std::wstring& /* xml */ )
{
  return BrokerToken();
}


//static
BrokerToken
BrokerToken::fromXml( const XMLElement& )
{
  return BrokerToken();
}

//static 
bool 
BrokerToken::toXml( const BrokerToken &, std::wstring& /* xml */ )
{
  return false;
}
