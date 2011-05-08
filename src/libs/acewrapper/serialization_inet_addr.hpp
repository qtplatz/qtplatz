// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <ace/INET_Addr.h>
#include <boost/serialization.hpp>

namespace boost {
  namespace serialization {

    template<class Archive>
	inline void save_construct_data(Archive& ar
									, const ACE_INET_Addr& addr
									, const unsigned int version ) {
	  // todo, not tested
	  ar << addr;
    }
    template<class Archive>
      void save(Archive& ar, ACE_INET_Addr& addr, const unsigned int version ) {
	  //ar << make_nvp("ipaddr",  addr.);
    }
    template<class Archive>
	inline void load_construct_data(Archive& ar
									, ACE_INET_Addr& addr
									, const unsigned int version ) {
	  
    }

  }
}
