//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

namespace device_emulator {

	class TXTSpectrum {
	public:
		TXTSpectrum() {}
		TXTSpectrum( const TXTSpectrum& );
		bool load( const std::string& );

	public:
		std::string filename_;
		std::vector<double> tarray_;
		std::vector<double> iarray_;
		unsigned long sampInterval_;
		unsigned long startDelay_;
		double minValue_;
		double maxValue_;
	};

}
