/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <map>
#include <string>

namespace adcontrols {
	class ProcessedDataset;
}

namespace fticr {

	class datafile : public adcontrols::datafile
		           , public adcontrols::LCMSDataset
				   , boost::noncopyable {
	public:
		datafile();

		//--------- implement adcontrols::datafile ----------------
		virtual void accept( adcontrols::dataSubscriber& );
		virtual boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const;
		virtual adcontrols::datafile::factory_type factory();
		virtual size_t getFunctionCount() const;
		virtual size_t getSpectrumCount( int fcn = 0 ) const;
		virtual size_t getChromatogramCount() const;
		virtual bool getTIC( int fcn, adcontrols::Chromatogram& ) const;
		virtual bool getSpectrum( int fcn, int idx, adcontrols::MassSpectrum& ) const;
		//<-------------------------------------
		bool _open( const std::wstring&, bool );
		static bool is_valid_datafile( const std::wstring& );
	private:
		std::wstring filename_; // root directory name
		boost::scoped_ptr< adcontrols::ProcessedDataset> processedDataset_;
		std::map< std::string, std::string > acqu_;
	};

}

#endif // DATAFILE_HPP
