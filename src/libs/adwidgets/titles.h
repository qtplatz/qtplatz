// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TITLES_H
#define TITLES_H

namespace SAGRAPHICSLib {
    struct ISADPTitles;
}

namespace adwidgets {
  namespace ui {

      class Title;

	  class Titles  {
	  public:
		  ~Titles();
		  Titles( SAGRAPHICSLib::ISADPTitles * pi = 0 );
		  Titles( const Titles& );

		  // LPUNKNOWN get__NewEnum();
		  Title item(long Index);
		  long count() const;
		  Title add();
		  void remove(long Index);
		  void clear();
		  bool visible() const;
		  void visible(bool newValue);
		  unsigned long color() const;
		  void color(unsigned long newValue);
		  // LPDISPATCH get_TitleFont();
		  // void put_TitleFont(LPDISPATCH newValue);
		  // LPDISPATCH get_SubtitleFont();
		  // void put_SubtitleFont(LPDISPATCH newValue);
		  long alignment();
		  void alignment(long newValue);
	  private:
          SAGRAPHICSLib::ISADPTitles * pi_;
	  };

  }
}

#endif // TITLES_H
