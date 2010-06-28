// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef ACEWRAPPER_H
#define ACEWRAPPER_H

namespace acewrapper {

  class instance_manager {
    ~instance_manager();
    instance_manager();
    static instance_manager * instance_;
  public:
    static void initialize();
    static void dispose();
  };
}

#endif // ACEWRAPPER_H
