// This is a -*- C++ -*- header.
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
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC Project
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
