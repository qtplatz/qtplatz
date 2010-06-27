// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef BROKERSESSION_H
#define BROKERSESSION_H

#include <string>
class BrokerToken;
class BrokerConfig;


class BrokerSession {
public:
  BrokerSession();
  BrokerSession( const BrokerConfig&, const BrokerToken& );

  bool disconnect();
  bool connect( const BrokerConfig, const std::wstring& user, const std::wstring& pass, 
		const std::wstring& connString, const std::wstring& revision = L"" );
};

#endif // BROKERSESSION_H
