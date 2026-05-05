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

#include "chromatogram.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <map>
#include <string>
#include <tuple>

namespace adcontrols {
	class ProcessedDataset;
}

namespace portfolio { class Portfolio; }
namespace shrader { class lrpfile; }

namespace shrader {

	class datafile : public adcontrols::datafile
		           , public adcontrols::LCMSDataset
                   , public std::enable_shared_from_this< datafile > {
        datafile& operator=( const datafile & ) = delete;
        datafile( const datafile& ) = delete;
	public:
		~datafile();
		datafile();

		//--------- implement adcontrols::datafile ----------------
		void accept( adcontrols::dataSubscriber& ) override;
		boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const override;
        boost::any fetch( const std::string& path, const std::string& dataType ) const override;
        adcontrols::datafile::factory_type factory() override;

        bool export_rawdata( const adcontrols::datafile& ) const override;

        // LCMSDataset
		size_t getFunctionCount() const override;
		size_t getSpectrumCount( int fcn = 0 ) const override;
		size_t getChromatogramCount() const override;
		bool getTIC( int fcn, adcontrols::Chromatogram& ) const override;
		bool getSpectrum( int fcn, size_t idx, adcontrols::MassSpectrum&, uint32_t objid ) const override;
        size_t posFromTime( double ) const override;
		double timeFromPos( size_t ) const override;
		bool getChromatograms( const std::vector< std::tuple<int, double, double> >&
                               , std::vector< adcontrols::Chromatogram >&
                               , std::function< bool (long curr, long total ) > /* progress */
                               , int begPos = 0
                               , int endPos = (-1) ) const override;
		bool hasProcessedSpectrum( int /* fcn */, int /* idx */) const override { return false; }

        // v3 data support
        size_t dataReaderCount() const override;
        const adcontrols::DataReader * dataReader( size_t idx ) const override;
        const adcontrols::DataReader * dataReader( const boost::uuids::uuid& ) const override;
        std::vector < std::shared_ptr< adcontrols::DataReader > > dataReaders( bool allPossible = false ) const override;
        // AcquiredDataset
        int dataformat_version() const override { return 3; }

		//<-------------------------------------
		bool lrp_open( const std::filesystem::path&, bool );
		static bool is_valid_datafile( const std::filesystem::path& );

        std::shared_ptr< const lrpfile > lrpfile() const;
	private:
        class impl;
        std::unique_ptr< impl > impl_;
	};

}

#endif // DATAFILE_HPP
