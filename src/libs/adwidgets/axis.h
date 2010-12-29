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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef AXIS_H
#define AXIS_H

#include <string>

namespace SAGRAPHICSLib {
    struct ISADPAxis;
}

namespace adwidgets {
  namespace ui {

    class Axis  {
    public:
		~Axis();
		Axis( SAGRAPHICSLib::ISADPAxis * pi = 0 );
		Axis( const Axis& );

		bool visible() const;
		void visible(bool newValue);
		double minimum() const;
		void minimum(double newValue);
		double maximum() const;
		void maximum(double newValue);
		double zoomMinimum() const;
		double zoomMaximum() const;
		long tickComputeStyle() const;
		void tickComputeStyle(long newValue);
		double majorTickIncrement() const;
		void majorTickIncrement(double newValue);
		double minorTickIncrement() const;
		void minorTickIncrement(double newValue);
		long majorTickStyle() const;
		void majorTickStyle(long newValue);
		long minorTickStyle() const;
		void minorTickStyle(long newValue);
		long tickLabelFormat() const;
		void tickLabelFormat(long newValue);
		short tickLabelDecimals() const;
		void tickLabelDecimals(short newValue);
		unsigned long color() const;
		void color(unsigned long newValue);
		std::wstring text() const;
		void text(const std::wstring& newValue);
		// LPDISPATCH Font();
		long orientation() const;
		long scaleStyle() const;
		void scaleStyle(long newValue);
		//LPDISPATCH Ticks();
		double firstTickValue() const;
		void firstTickValue(double newValue);
		long labelOrientation() const;
		void labelOrientation(long newValue);
		short lineWidth() const;
		void lineWidth(short newValue);
		long lineStyle() const;
		void lineStyle(long newValue);
		bool enableMarker() const;
		void enableMarker(bool newValue);
		double markerPosition() const;
		void markerPosition(double newValue);
		short tickLabelMaximumLength() const;
		void tickLabelMaximumLength(short newValue);
		bool tickLabelsVisible() const;
		void tickLabelsVisible(bool newValue);
	private:
        SAGRAPHICSLib::ISADPAxis * pi_;
	};
  }
}

#endif // AXIS_H
