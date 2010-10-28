// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

namespace adcontrols {

    class Visitor;

    class Acceptor {
    public:
        virtual ~Acceptor(void);
        virtual bool accept( Visitor& visitor );
    };

}
