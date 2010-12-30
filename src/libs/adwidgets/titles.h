// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

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
		  size_t count() const;
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
		  Title operator [] ( int index );
	  private:
          SAGRAPHICSLib::ISADPTitles * pi_;
	  };

  }
}

#endif // TITLES_H
