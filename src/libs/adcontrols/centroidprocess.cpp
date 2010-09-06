//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "centroidprocess.h"
#include "centroidmethod.h"
#include "import_sacontrols.h"

using namespace adcontrols;

namespace adcontrols {
	namespace internal {
        
		class SAMassSpectrum {
		public:
			void copy( SACONTROLSLib::ISAMassSpectrum5*, const MassSpectrum& );
			void copy( MassSpectrum&, SACONTROLSLib::ISAMassSpectrum5* );
		};

	}
}

CentroidProcess::~CentroidProcess(void)
{
}

CentroidProcess::CentroidProcess(void)
{
}

bool
CentroidProcess::operator()( const CentroidMethod& method, const MassSpectrum& profile )
{
	CComPtr<SACONTROLSLib::ISAMSPeakDetect> pi;
	if ( pi.CoCreateInstance( SACONTROLSLib::CLSID_SAMSPeakDetect ) != S_OK )
		return false;

    setup( pi, method );

	return true;
}

void
CentroidProcess::setup( SACONTROLSLib::ISAMSPeakDetect* pi, const CentroidMethod& method )
{
	if ( method.centroidAreaIntensity() )
		pi->SetCentroidAreaIntensity();
	else
		pi->SetCentroidHeightIntensity();

	switch(method.peakWidthMethod()) {
	case CentroidMethod::ePeakWidthTOF:
		pi->SetTOFPeakWidth( method.rsTofInDa(), method.rsTofAtMz() );
		break;
	case CentroidMethod::ePeakWidthProportional:
		pi->SetppmPeakWidth( method.rsPropoInPpm() );
		break;
	case CentroidMethod::ePeakWidthConstant:
		pi->SetDaPeakWidth( method.rsConstInDa() );
		break;
	}
	pi->SetBaselineWidth( method.baselineWidth() );
	pi->SetAttenuation( method.attenuation() );
	pi->SetPeakCentroidFraction( method.peakCentroidFraction() );
}

using namespace adcontrols::internal;

void
SAMassSpectrum::copy( SACONTROLSLib::ISAMassSpectrum5* pi, const MassSpectrum& ms)
{
}

void
SAMassSpectrum::copy( MassSpectrum& ms, SACONTROLSLib::ISAMassSpectrum5* )
{
}

