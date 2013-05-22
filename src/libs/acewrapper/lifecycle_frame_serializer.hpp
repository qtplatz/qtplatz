// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

// #include <adportable/lifecycleprotocol.h>

namespace adportable {
    namespace protocol {
        struct LifeCycleFrame;
        struct LifeCycle_Hello;
        struct LifeCycle_SYN;
        struct LifeCycle_SYN_Ack;
        struct LifeCycle_Data;
        struct LifeCycle_DataAck;
        struct LifeCycle_Close;
    }
}

/*
namespace acewrapper {
	class OutputCDR;
	class InputCDR;
}
*/

class ACE_Message_Block;
class ACE_InputCDR;
class ACE_OutputCDR;

namespace acewrapper {

  class lifecycle_frame_serializer {
  public:
    lifecycle_frame_serializer();

    template<class T> static ACE_Message_Block * pack( const T& );
	template<class T> static bool pack( ACE_OutputCDR&, const T& );
	// template<class T> static bool pack( OutputCDR&, const T& );

    template<class T> static bool unpack( ACE_Message_Block *, adportable::protocol::LifeCycleFrame&, T& );
	template<class T> static bool unpack( ACE_InputCDR&, adportable::protocol::LifeCycleFrame&, T& );
    // template<class T> static bool unpack( InputCDR&, adportable::protocol::LifeCycleFrame&, T& );
  };

}


