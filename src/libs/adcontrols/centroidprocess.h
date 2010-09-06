// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/smart_ptr.hpp>

namespace SACONTROLSLib {
	struct ISAMSPeakDetect;
	struct ISAMassSpectrum;
}

namespace adcontrols {

    class CentroidMethod;
    class MassSpectrum;

    namespace internal {
        class CentroidProcessImpl;
    }

	class ADCONTROLSSHARED_EXPORT CentroidProcess {
	public:
		CentroidProcess(void);
		~CentroidProcess(void);
        bool operator()( const CentroidMethod&, const MassSpectrum& );
        bool getCentroidSpectrum( MassSpectrum& );

	private:
        internal::CentroidProcessImpl* pImpl_;
		static void setup( SACONTROLSLib::ISAMSPeakDetect*, const CentroidMethod& );
	};

}
