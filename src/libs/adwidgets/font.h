// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef FONT_H
#define FONT_H

struct IDispatch;

namespace adil {
  namespace ui {

    class Font  {
    public:
		~Font();
		Font( IDispatch * pi = 0 );
		Font( const Font& );
    private:
		IDispatch * pi_;
    };

  }
}

#endif // FONT_H
