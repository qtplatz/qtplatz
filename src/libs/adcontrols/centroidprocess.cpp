//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "centroidprocess.h"
#include "centroidmethod.h"
#include "import_sacontrols.h"
#include "massspectrum.h"
#include "mspeakinfoitem.h"
#include "description.h"
#include <vector>
#include <boost/smart_ptr.hpp>
#include <sstream>

/*
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/base_object.hpp>
*/
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>


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
			void clear();
            void setup( const CentroidMethod& );
            void setup( const MassSpectrum& );
			void copy( MassSpectrum& );
			const CentroidMethod& method() const { return method_; }

			// result
            std::vector< MSPeakInfoItem > info_;
		private:
			MassSpectrum clone_;
			CentroidMethod method_;
			Description desc_;
        };

	}
}

CentroidProcess::~CentroidProcess(void)
{
    delete pImpl_;
}

CentroidProcess::CentroidProcess(void) : pImpl_( new internal::CentroidProcessImpl() )
{
}

CentroidProcess::CentroidProcess( const CentroidMethod& method)
  : pImpl_( new internal::CentroidProcessImpl() )
{
	pImpl_->setup( method );
}

bool
CentroidProcess::operator()( const CentroidMethod& method, const MassSpectrum& profile )
{
	pImpl_->setup( method );
	return (*this)( profile );
}

bool
CentroidProcess::operator()( const MassSpectrum& profile )
{
    pImpl_->clear();
	pImpl_->setup( profile );

	CComPtr<SACONTROLSLib::ISAMSPeakDetect2> pi;
	if ( pi.CoCreateInstance( SACONTROLSLib::CLSID_SAMSPeakDetect ) != S_OK )
		return false;
	setup( pi, pImpl_->method() );

    CComPtr<SACONTROLSLib::ISAMassSpectrum5> pims;
    if ( pims.CoCreateInstance( SACONTROLSLib::CLSID_SAMassSpectrum ) != S_OK )
        return false;

    internal::SAMassSpectrum::copy( pims, profile );
    do {
        try {
            CComQIPtr< SACONTROLSLib::ISAMassSpectrum > piMS = pims;
            if ( pi->Detect( piMS ) != S_OK )
                return false;
        } catch ( _com_error & ) {
            return false;
        }
    } while(0);

	CComPtr< SACONTROLSLib::ISAMSPeakInformation2 > piInfo = pi->PeakInformation2;
    size_t nSize = piInfo->Count;

    if ( nSize ) {
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
	pImpl_->copy( ms );

	size_t nSize;
	if ( pImpl_ && ( nSize = pImpl_->info_.size() ) ) {

		ms.resize( nSize );
        ms.setCentroid( adcontrols::CentroidPeakAreaWaitedMass );
		bool is_area = pImpl_->method().centroidAreaIntensity();

        for ( size_t i = 0; i < nSize; ++i ) {
			ms.setIntensity( i, is_area ? pImpl_->info_[i].area() : pImpl_->info_[i].height() );
            ms.setMass( i, pImpl_->info_[i].mass() );
			// todo : tof array should be calculated
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
SAMassSpectrum::copy( MassSpectrum&, SACONTROLSLib::ISAMassSpectrum5* )
{
}

/////////////////////////

using namespace adcontrols::internal;

void
CentroidProcessImpl::clear()
{
	info_.clear();
}

void
CentroidProcessImpl::setup( const CentroidMethod& method )
{
    method_ = method;
	desc_ = adcontrols::Description( L"CentroidProcess", L"Centroid" );

	std::wostringstream o;
	boost::archive::xml_woarchive ar( o );
	ar << boost::serialization::make_nvp("CentroidMethod", method);
	desc_.xml( o.str() );
}

void
CentroidProcessImpl::setup( const MassSpectrum& profile )
{
    clone_.clone( profile, false ); // keep meta data
}

void
CentroidProcessImpl::copy( MassSpectrum& ms )
{
	ms.clone( clone_, false );
	ms.addDescription( desc_ );
}