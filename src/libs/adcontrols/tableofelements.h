// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TABLEOFELEMENTS_H
#define TABLEOFELEMENTS_H

namespace adcontrols {

  class TableOfElements {
    TableOfElements();
  public:
    TableOfElements * instance();

  private:
    static TableOfElements * instance_;

  };

}

#endif // TABLEOFELEMENTS_H
