// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "ConvertUTF.h"
#include <string>

namespace adportable {

  class utf {
  public:
      static std::basic_string<UTF8> to_utf8( const UTF16 * );
      static std::basic_string<UTF8> to_utf8( const UTF32 * );
      static std::basic_string<UTF16> to_utf16( const UTF8 * );
      static std::basic_string<UTF16> to_utf16( const UTF32 * );
      static std::basic_string<UTF32> to_utf32( const UTF8 * );
      static std::basic_string<UTF32> to_utf32( const UTF16 * );
  };

}

