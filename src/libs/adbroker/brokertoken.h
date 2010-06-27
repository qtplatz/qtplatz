// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////
#ifndef BROKERTOKEN_H
#define BROKERTOKEN_H

#include <string>

class XMLElement;

class BrokerToken {
 public:
  ~BrokerToken();
  BrokerToken();
  BrokerToken( const BrokerToken& );

  const std::wstring& connDate() const;
  const std::wstring& connString() const;
  const std::wstring& terminalId() const;
  const std::wstring& sessionGuid() const;
  const std::wstring& ownerId() const;
  const std::wstring& ownerName() const;
  const std::wstring& revision() const;
  
  static BrokerToken Create( const std::wstring& ownerId
			     , const std::wstring& ownerName
			     , const std::wstring& sessionGuid
			     , const std::wstring& terminalId
			     , const std::wstring& connString
			     , const std::wstring& revision );
  
  static BrokerToken fromXml( const std::wstring& xml );
  static BrokerToken fromXml( const XMLElement& );
  static bool toXml( const BrokerToken &, std::wstring& xml );
  
private:
  std::wstring connString_;
  std::wstring terminalId_;
  std::wstring sessionGuid_;
  std::wstring ownerId_;
  std::wstring ownerName_;
  std::wstring connDate_;
  std::wstring revision_;

};

#endif // BROKERTOKEN_H
