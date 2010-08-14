// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adinterface/receiverS.h"

class receiver_i : public virtual POA_Receiver {
public:
    receiver_i(void);
    ~receiver_i(void);

    //void message( Receiver::eMESSAGE msg, CORBA::ULong st );
    //void eventLog( const Receiver::EventLogData& log );
    //void shutdown();
};
