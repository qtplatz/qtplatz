// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <adplugin/ifactory.h>

namespace qtwidgets {

    class factory : public adplugin::ifactory {
    public:
        virtual QWidget * create_widget( const wchar_t * iid, QWidget * parent );
        virtual QObject * create_object( const wchar_t * iid, QObject * parent );
        virtual void release();
        
    };
}

