// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>
#include <QString>

namespace qtwrapper {

  class qstring {
    QString q_;
  public:
    qstring( const std::wstring& );
    inline operator QString& () { return q_; }
  };

}

