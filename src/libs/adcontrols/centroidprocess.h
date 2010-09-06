// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

namespace SACONTROLSLib {
	struct ISAMSPeakDetect;
	struct ISAMassSpectrum;
}

namespace adcontrols {

    class CentroidMethod;
    class MassSpectrum;

	class CentroidProcess {
	public:
		CentroidProcess(void);
		~CentroidProcess(void);
		bool operator()( const CentroidMethod&, const MassSpectrum& );

	private:
		static void setup( SACONTROLSLib::ISAMSPeakDetect*, const CentroidMethod& );
	};

}
