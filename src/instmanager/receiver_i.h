#pragma once

#include "interface/receiverS.h"

class receiver_i : public virtual POA_Receiver {
public:
    receiver_i(void);
    ~receiver_i(void);

    //void message( Receiver::eMESSAGE msg, CORBA::ULong st );
    //void eventLog( const Receiver::EventLogData& log );
    //void shutdown();
};
