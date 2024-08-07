/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@MS-Cheminformatics.com
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

#ifndef DATAFILE_HPP
#define DATAFILE_HPP

#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/processeddataset.hpp>
#include <memory>
#import <CompassXtractMS.dll>

namespace adcontrols { class Chromatogram; class MassSpectrum; }

namespace compassxtract {

	class datafile : public adcontrols::datafile
                   , public adcontrols::LCMSDataset {
        datafile( const datafile& ) = delete;
        datafile& operator = ( const datafile& ) = delete;
	public:
		datafile();

		//--------- implement adcontrols::datafile ----------------
		const std::wstring& filename() const override;
		void accept( adcontrols::dataSubscriber& ) override;
		boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const override;
		adcontrols::datafile::factory_type factory() override;
		size_t getFunctionCount() const override;
		size_t getSpectrumCount( int fcn = 0 ) const override;
		size_t getChromatogramCount() const override;
		bool getTIC( int fcn, adcontrols::Chromatogram& ) const override;
		bool getSpectrum( int fcn, size_t idx, adcontrols::MassSpectrum&, uint32_t objId ) const override;
		size_t posFromTime( double ) const override;
		double timeFromPos( size_t ) const override;
		bool getChromatograms( const std::vector< std::tuple<int, double, double> >&
			                         , std::vector< adcontrols::Chromatogram >&
									 , std::function< bool (long curr, long total ) > progress
									 , int begPos = 0
									 , int endPos = (-1) ) const override { (void)begPos; (void)endPos; return false; }
		bool hasProcessedSpectrum( int /* fcn */, int /* idx */ ) const override;
        uint32_t findObjId( const std::wstring& traceId ) const override;
        
		//<-------------------------------------
		bool _open( const std::wstring&, bool );
		static bool is_valid_datafile( const std::wstring& );
	private:
		bool getTIC();
        bool import( adcontrols::MassSpectrum&, EDAL::IMSSpectrum2Ptr, EDAL::SpectrumTypes ) const;

		std::wstring filename_; // root directory name
		std::unique_ptr< adcontrols::ProcessedDataset> processedDataset_;
        std::shared_ptr< adcontrols::Chromatogram > pTIC_;
        std::shared_ptr< adcontrols::MassSpectrum > refMS_;
		EDAL::IMSAnalysis2Ptr pAnalysis_;
	};

}

#endif // DATAFILE_HPP
