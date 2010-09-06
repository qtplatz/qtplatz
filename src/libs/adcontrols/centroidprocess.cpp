//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "centroidprocess.h"
#include "centroidmethod.h"
#include "import_sacontrols.h"
#include "massspectrum.h"
#include "mspeakinfoitem.h"
#include <vector>
#include <boost/smart_ptr.hpp>

using namespace adcontrols;

namespace adcontrols {

    class MSPeakInfoItem;

	namespace internal {
        
		class SAMassSpectrum {
		public:
            static void copy( SACONTROLSLib::ISAMassSpectrum5*, const MassSpectrum& );
            static void copy( MassSpectrum&, SACONTROLSLib::ISAMassSpectrum5* );
		};

        class CentroidProcessImpl {
        public:
            CentroidProcessImpl() {}
            std::vector< MSPeakInfoItem > info_;
        };

	}
}

CentroidProcess::~CentroidProcess(void)
{
    delete pImpl_;
}

CentroidProcess::CentroidProcess(void) : pImpl_(0)
{
}

bool
CentroidProcess::operator()( const CentroidMethod& method, const MassSpectrum& profile )
{
    if ( pImpl_ ) {
        delete pImpl_;
        pImpl_ = 0;
    }
	CComPtr<SACONTROLSLib::ISAMSPeakDetect2> pi;
	if ( pi.CoCreateInstance( SACONTROLSLib::CLSID_SAMSPeakDetect ) != S_OK )
		return false;
    setup( pi, method );

    CComPtr<SACONTROLSLib::ISAMassSpectrum5> pims;
    if ( pims.CoCreateInstance( SACONTROLSLib::CLSID_SAMassSpectrum ) != S_OK )
        return false;

    internal::SAMassSpectrum::copy( pims, profile );
    do {
        CComQIPtr< SACONTROLSLib::ISAMassSpectrum > piMS = pims;
        if ( pi->Detect( piMS ) != S_OK )
            return false;
    } while(0);

    CComPtr< SACONTROLSLib::ISAMSPeakInformation2 > piInfo = pi->PeakInformation2;
    size_t nSize = piInfo->Count;

    if ( nSize ) {
        pImpl_ = new internal::CentroidProcessImpl();
        CComPtr< SACONTROLSLib::ISAMSPeakInformationItem2 > piItem;
        for ( size_t i = 0; i < nSize; ++i ) {
            piItem = piInfo->Item[ i + 1 ];
            double mass = piItem->GetPeakAreaWeightedMass();
            double area = piItem->GetPeakArea();
            double height = piItem->GetPeakHeight();
            double hh = piItem->GetPeakWidthHH();
            pImpl_->info_.push_back( MSPeakInfoItem( mass, area, height, hh ) );
        }
    }
    return true;
}

bool
CentroidProcess::getCentroidSpectrum( MassSpectrum& ms )
{
    if ( pImpl_ && pImpl_->info_.size() ) {
        size_t nSize = pImpl_->info_.size();
        ms.resize( nSize );
        ms.setCentroid( adcontrols::CentroidPeakAreaWaitedMass );
        for ( size_t i = 0; i < nSize; ++i ) {
            ms.setIntensity( i, pImpl_->info_[i].area() );
            ms.setMass( i, pImpl_->info_[i].mass() );
        }
        return true;
    }
    return false;
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
    pi->Count = ms.size();
    pi->IsCentroid = ms.isCentroid() ? VARIANT_TRUE : VARIANT_FALSE;
    pi->SetIntensityArrayDirect2( reinterpret_cast<IUnknown *>(const_cast<double *>(ms.getIntensityArray())) );
    pi->SetMassArrayDirect2( reinterpret_cast<IUnknown *>(const_cast<double *>(ms.getMassArray())) );
}

void
SAMassSpectrum::copy( MassSpectrum& ms, SACONTROLSLib::ISAMassSpectrum5* )
{
}

