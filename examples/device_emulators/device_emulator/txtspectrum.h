// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#pragma once

#include <string>
#include <vector>

namespace device_emulator {

	class TXTSpectrum {
	public:
		TXTSpectrum() {}
		TXTSpectrum( const TXTSpectrum& );
		bool load( const std::string& );
		bool load3( const std::string& );

	public:
		std::string filename_;
		std::vector<double> tarray_;
		std::vector<double> iarray_;
        std::vector<double> marray_;
		unsigned long sampInterval_;
		unsigned long startDelay_;
		double minValue_;
		double maxValue_;
	};

}
