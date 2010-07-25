// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

class signal_handler {
public:
  static int pidChild;
  static int pidParent;
  static int respawn_flag;

  static void sigint(int);
};

