// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ROLEESI_H
#define ROLEESI_H

#include "device_state.h"

class RoleESI : public device_state {
public:
    ~RoleESI();
    RoleESI();
    RoleESI( const RoleESI& );
    bool operator == ( const RoleESI& ) const { return true; }
    const char * deviceType() const { return "ionsouce/esi"; }
};

#endif // ROLEESI_H
