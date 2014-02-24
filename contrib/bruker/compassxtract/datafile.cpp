/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "datafile.hpp"
#include "safearray.hpp"
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adportable/array_wrapper.hpp>
#include <adlog/logger.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>

namespace compassxtract {

	struct filename_d {
		static boost::filesystem::path root_name( const boost::filesystem::path& path ) {
			boost::filesystem::path rpath( path );
			if ( boost::filesystem::is_regular_file( path ) ) {
				rpath = path.parent_path();
				if ( boost::filesystem::is_directory( rpath ) && rpath.extension() == L".d" )
					return rpath;
			}
			return rpath;
		}
	};

	struct date_t {
		static boost::posix_time::ptime ptime( const SYSTEMTIME & st ) {
			try {
				boost::gregorian::date d( st.wYear, st.wMonth, st.wDay );
				boost::posix_time::time_duration td( st.wHour, st.wMinute, st.wSecond );
				return boost::posix_time::ptime( d, td );
			} catch ( boost::gregorian::bad_day_of_month& ) {
				return boost::posix_time::ptime();
			}
		}
	};

}

using namespace compassxtract;

datafile::datafile()
{
}

// virtual
const std::wstring&
datafile::filename() const
{
	return filename_;
}

//virtual
void
datafile::accept( adcontrols::dataSubscriber& sub )
{
    // AcquireDataset <LCMSDataset>
	sub.subscribe( *this );

    // subscribe processed dataset
	if ( processedDataset_ ) 
		sub.subscribe( *processedDataset_ );
}

// virtual
boost::any
datafile::fetch( const std::wstring& path, const std::wstring& dataType ) const
{
    (void)path;
	(void)dataType;
	boost::any any;

	return any;
}

//virtual
adcontrols::datafile::factory_type
datafile::factory()
{ 
	return 0;
}

//virtual
size_t
datafile::getFunctionCount() const
{
	return 2;
}

//virtual
size_t
datafile::getSpectrumCount( int /* fcn */ ) const
{
	try {
		EDAL::IMSSpectrumCollectionPtr pSpectra = pAnalysis_->GetMSSpectrumCollection();
		return pSpectra->Count;
	} catch(_com_error& ex ) {
		::MessageBox(NULL, ex.ErrorMessage(), L"compassxtract::datafile::getSpectrumCount", MB_OK);
	}
	return 0;
}

//virtual
size_t
datafile::getChromatogramCount() const
{
	return 0;
}

bool
datafile::hasProcessedSpectrum( int /* fcn */, int /* idx */) const
{ 
	return true;
}

uint32_t
datafile::findObjId( const std::wstring& traceId ) const
{ 
    if ( traceId == L"MS.PROFILE" )
        return 1;
    if ( traceId == L"MS.CENTROID" )
        return 2;
	return 0;
}

#if defined _MSC_VER
# pragma warning(disable:4482)
#endif
//virtual
bool
datafile::getSpectrum( int fcn, int pos, adcontrols::MassSpectrum& ms, uint32_t objId ) const
{
	(void)fcn;

	try {
		EDAL::IMSSpectrumCollectionPtr pSpectra = pAnalysis_->GetMSSpectrumCollection();
		EDAL::IMSSpectrumPtr pSpectrum = pSpectra->GetItem( pos + 1 ); // 1-origin

		if ( pSpectrum->Polarity == EDAL::SpectrumPolarity::IonPolarity_Negative )
			ms.setPolarity( adcontrols::MS_POLARITY::PolarityNegative );
		else if ( pSpectrum->Polarity == EDAL::SpectrumPolarity::IonPolarity_Positive )
			ms.setPolarity( adcontrols::MS_POLARITY::PolarityPositive );
		else
			ms.setPolarity( adcontrols::MS_POLARITY::PolarityIndeterminate );

		adcontrols::MSProperty prop = ms.getMSProperty();
		prop.setTimeSinceInjection( static_cast< unsigned long >( pSpectrum->RetentionTime /* sec */ * 1.0e6 ) ); // usec
        ms.setMSProperty( prop ); // <- end of prop set

		_variant_t vMasses, vIntens;
        if ( objId <= 1 ) {
			pSpectrum->GetMassIntensityValues( EDAL::SpectrumType_Profile, &vMasses, &vIntens );
			ms.setCentroid( adcontrols::CentroidNone );  // profile
        } else { // objId should be 2
			pSpectrum->GetMassIntensityValues( EDAL::SpectrumType_Line, &vMasses, &vIntens );
			ms.setCentroid( adcontrols::CentroidNative );
		}

		SafeArray sa_masses( vMasses );
        ms.resize( sa_masses.size() );
        ms.setMassArray( reinterpret_cast< const double *>( sa_masses.p() ) );
    
		SafeArray sa_intensities( vIntens );
        ms.setIntensityArray( reinterpret_cast< const double *>( sa_intensities.p() ) );

        ms.setAcquisitionMassRange( ms.getMass( 0 ), ms.getMass( ms.size() - 1 ) );

		return true;
	} catch(_com_error& ex ) {
		ADERROR() << std::wstring( ex.ErrorMessage() );
		return false;
	}
	return false;
}

/////////////////////////

bool
datafile::_open( const std::wstring& filename, bool )
{
	filename_ = filename;
	boost::filesystem::path rpath = filename_d::root_name( filename );

	portfolio::Portfolio portfolio;
	portfolio.create_with_fullpath( rpath.wstring() );
	portfolio::Folder fchromatograms = portfolio.addFolder( L"Chromatograms" );
	portfolio::Folder fspectra = portfolio.addFolder( L"Spectra" );

    HRESULT hr;
	if ( ( hr = pAnalysis_.CreateInstance( L"EDAL.MSAnalysis" ) ) == S_OK ) {
		try  {
			if ( ( hr = pAnalysis_->Open( _bstr_t( rpath.wstring().c_str() ) ) ) == S_OK ) {

				filename_ = rpath.wstring();

				// this is basic information retrievable
				// right from the analysis
                _bstr_t operatorName, analysisName, dateIso, analysisDesc;
                operatorName = pAnalysis_->GetOperatorName();
                analysisName = pAnalysis_->GetAnalysisName();
                dateIso      = pAnalysis_->GetAnalysisDateTimeIsoString();
				analysisDesc = pAnalysis_->GetAnalysisDescription();

                SYSTEMTIME st;
				VariantTimeToSystemTime( pAnalysis_->GetAnalysisDateTime(), &st );
				std::string creationDate = boost::posix_time::to_simple_string( date_t::ptime( st ) );
                
				// CString creationDate = dt.Format( _T("%m/%d/%Y %H:%M") );
                std::cout << "Analysis:     " << analysisName << std::endl;
                std::cout << "Operator:     " << operatorName << std::endl;
                std::cout << "Date/TimeIso: " << dateIso << std::endl;
				std::cout << "Date/Time:    " << creationDate << std::endl;
				std::cout << "Description:  " << analysisDesc << std::endl;

				// get a pointer to the interface of this analysis' spectrum collection
				// the spectrum collection can then be used to retrieve indivudual spectrums
				EDAL::IMSSpectrumCollectionPtr pSpectra = pAnalysis_->GetMSSpectrumCollection();
				size_t n = pSpectra->Count;
				(void)n;
            }
        } catch(_com_error& ex ) {
			ADERROR() << ex.ErrorMessage();
			return false;
        }
    }

	processedDataset_.reset( new adcontrols::ProcessedDataset );
	processedDataset_->xml( portfolio.xml() );
    return true;
}

/*
FirstLineIntensity         float Intensity of the first line saved for each spectrum
HighResolutionMass         float High resolution mass of each spectrum
IonPlo1arity                int   Polarity of each spectrum (0 = positive, 1 = negative, 2 = both, 255 = unknown)
MaxIntensity               float Maximum intensity of each spectrum
MSLevel                    int   The MS level of each spectrum (MS = 1, MSMS = 2, �)
MSPrecursor                float Each MSMS spectrum�s precursor mass
MZMeasurementIntervalEnd   float Measured mass range, last values
MZMeasurementIntervalStart float Measured mass range, first values
RetentionTime              float The retention time of each spectrum in seconds
SumIntensity               float Sum of all intensities (aka Total Ion Current)
*/

// virtual
bool
datafile::getTIC( int fcn, adcontrols::Chromatogram& c ) const
{
	(void)fcn;

	CComBSTR strSumIntensities( L"SumIntensity" );
	CComBSTR strRetentionTime(  L"RetentionTime" );

	if( pAnalysis_->HasAnalysisData( &strSumIntensities ) )	{
		_variant_t vIntens = pAnalysis_->GetAnalysisData( &strSumIntensities );
		SafeArray sIntens( vIntens );
		c.resize( sIntens.size() );
		c.setIntensityArray( reinterpret_cast< const double *>( sIntens.p() ) );
	}

	if( pAnalysis_->HasAnalysisData( &strRetentionTime ) )	{
		// array of seconds
		_variant_t vTimes = pAnalysis_->GetAnalysisData( &strRetentionTime );
		SafeArray sTimes( vTimes );
		c.setTimeArray( reinterpret_cast< const double *>( sTimes.p() ) );
	}
	return true;
}

bool
datafile::getTIC()
{
	pTIC_.reset( new adcontrols::Chromatogram() );
	return getTIC( 0, *pTIC_ );
}

size_t
datafile::posFromTime( double minutes ) const
{
	using adportable::array_wrapper;
	if ( ! pTIC_ )
		const_cast< datafile *>(this)->getTIC();
	double t = adcontrols::Chromatogram::toSeconds( minutes );
	array_wrapper< const double > times( pTIC_->getTimeArray(), pTIC_->size() );
	array_wrapper< const double >::iterator it = std::lower_bound( times.begin(), times.end(), t );
	long pos = std::distance( times.begin(), it );
	return pos;
}

double
datafile::timeFromPos( size_t pos ) const
{
	if ( pTIC_ && pTIC_->size() > pos ) {
		return pTIC_->getTimeArray()[ pos ];
	}
	return 0;
}


//static
bool
datafile::is_valid_datafile( const std::wstring& filename )
{
	boost::filesystem::path rpath = filename_d::root_name( filename );

	EDAL::IMSAnalysis2Ptr pAnalysis;

	// use and check for a successful creation with HRESULT
	HRESULT hr = pAnalysis.CreateInstance("EDAL.MSAnalysis");
	if( SUCCEEDED(hr) ) {
		try  {
			// use open() with the path to open an analysis
			// and check for success
			hr = pAnalysis->Open( _bstr_t( rpath.wstring().c_str() ) );
			return hr == S_OK; // 
		} catch(_com_error& ex ) {
			ADERROR() << ex.ErrorMessage();
		}

/*
            if( SUCCEEDED(hr) )  {

				// this is basic information retrievable
				// right from the analysis
                _bstr_t operatorName, analysisName, dateIso, analysisDesc;
                operatorName = pAnalysis->GetOperatorName();
                analysisName = pAnalysis->GetAnalysisName();
                dateIso      = pAnalysis->GetAnalysisDateTimeIsoString();
				analysisDesc = pAnalysis->GetAnalysisDescription();

                COleDateTime dt = pAnalysis->GetAnalysisDateTime();
                CString creationDate = dt.Format( _T("%m/%d/%Y %H:%M") );

                std::cout << "Analysis:     " << analysisName << std::endl;
                std::cout << "Operator:     " << operatorName << std::endl;
                std::cout << "Date/TimeIso: " << dateIso << std::endl;
                std::cout << "Date/Time:    " << creationDate << std::endl;
				std::cout << "Description:  " << analysisDesc << std::endl;

				// get a pointer to the interface of this analysis' spectrum collection
				// the spectrum collection can then be used to retrieve indivudual spectrums
                EDAL::IMSSpectrumCollectionPtr pSpectra = pAnalysis->GetMSSpectrumCollection();

				// example for new (CXT 3.1) data access functions
				DumpHighestTICSpectrum( pAnalysis );

				// dump all the spectrum data
                DumpSpectra(pSpectra);
            }
*/
    }
	return false;
}

