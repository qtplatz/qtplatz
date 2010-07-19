// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef ROLEAVERAGER_H
#define ROLEAVERAGER_H

#include "device_state.h"

class RoleAverager : public device_state {
public:
    ~RoleAverager();
    RoleAverager();
    RoleAverager( const RoleAverager& );
    bool operator == ( const RoleAverager& ) const { return true; }
    const char * deviceType() const { return "averager"; }

    struct handleIt {
        virtual void operator()( bool ) const {}
    };
    // trigger disarmed after current averaging
    bool instruct_average_stop( handleIt& = handleIt() );  
    
    // trigger armed immediately
    bool instruct_average_start();  
};

#endif // ROLEAVERAGER_H
