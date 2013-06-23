// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#ifndef DATAFILE_H
#define DATAFILE_H

#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adfs/adfs.hpp>
#include <memory>

namespace adcontrols {
    class Chromatogram;
    class MassSpectrum;
    class ProcessedDataset;
}

namespace portfolio { class Portfolio; }

namespace addatafile {

    class datafile : public adcontrols::datafile
                   , public adcontrols::LCMSDataset { 
    public:
        ~datafile();
        datafile();
        
        bool open( const std::wstring& filename, bool readonly = false );
        // bool open_qtms( const std::wstring& filename, bool readonly = false );

        //--------- implement adcontrols::datafile ----------------
        void accept( adcontrols::dataSubscriber& ) override;
        boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const override;

        // create, modify and delete methods
        bool saveContents( const std::wstring&, const portfolio::Portfolio&, const adcontrols::datafile& ) override;
        bool saveContents( const std::wstring&, const portfolio::Portfolio& ) override;
        bool update( const std::wstring&, boost::any& ) override { return false; }

        adcontrols::datafile::factory_type factory() override { return 0; }

        // LCMSDataset
        size_t getFunctionCount() const override;
        size_t getSpectrumCount( int fcn = 0 ) const override;
        size_t getChromatogramCount() const override;
        bool getTIC( int fcn, adcontrols::Chromatogram& ) const override;
        bool getSpectrum( int fcn, int idx, adcontrols::MassSpectrum& ) const override;
		size_t posFromTime( double ) const override;
		bool getChromatograms( int fcn
			                         , const std::vector< std::pair<double, double> >&
			                         , std::vector< adcontrols::Chromatogram >&
									 , std::function< bool (long curr, long total ) > progress
									 , int begPos = 0
									 , int endPos = (-1) ) const override;
    private:
        bool loadContents( portfolio::Portfolio&, const std::wstring& query );
    private:
        bool mounted_;
        std::wstring filename_;
        adfs::portfolio dbf_;
		std::unique_ptr< adcontrols::ProcessedDataset > processedDataset_;
        //boost::any data_;
    };

}

#endif // DATAFILE_H
