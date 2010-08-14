// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adplugin_global.h"
#include <boost/any.hpp>

namespace adportable {
    class Configuration;
}

namespace adplugin {

    class ADPLUGINSHARED_EXPORT LifeCycle {
    public:
        virtual void OnCreate( const adportable::Configuration& ) = 0;
        virtual void OnInitialUpdate() = 0;
        virtual void OnUpdate( boost::any& ) {}
        virtual void OnUpdate( unsigned long ) {}
        virtual void OnFinalClose() = 0;
    };

}


