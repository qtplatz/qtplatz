// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/chromatogram.hpp>
#include <portfolio/portfolio.hpp>
#include <memory>
#include <map>

namespace adcontrols {
    class Chromatogram;
    class MassSpectrum;
    class ProcessedDataset;
}

namespace adtextfile {

    class TXTSpectrum;
    class TXTChromatogram;

    class datafile : public adcontrols::datafile
                   , public adcontrols::LCMSDataset { 
    public:
        ~datafile();
        datafile();
        
        bool open( const std::wstring& filename, bool readonly = false );

        //--------- implement adcontrols::datafile ----------------
        virtual void accept( adcontrols::dataSubscriber& );
        virtual boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const;

        virtual adcontrols::datafile::factory_type factory() { return 0; }

        // LCMSDataset
        virtual size_t getFunctionCount() const;
        virtual size_t getSpectrumCount( int fcn = 0 ) const;
        virtual size_t getChromatogramCount() const;
        virtual bool getTIC( int fcn, adcontrols::Chromatogram& ) const;
        virtual bool getSpectrum( int fcn, size_t pos, adcontrols::MassSpectrum&, uint32_t objid ) const;
		virtual size_t posFromTime( double ) const;
		double timeFromPos( size_t ) const;
		bool getChromatograms( const std::vector< std::tuple<int, double, double> >&
                               , std::vector< adcontrols::Chromatogram >&
                               , std::function< bool (long curr, long total ) > progress
                               , int /* begPos */
                               , int /* endPos */ ) const override { return false; }
        
    private:
		std::unique_ptr< adcontrols::ProcessedDataset > processedDataset_;
        std::map< std::wstring, adcontrols::MassSpectrumPtr > data_;
        std::map< std::wstring, adcontrols::ChromatogramPtr > chro_;
        bool prepare_portfolio( const TXTSpectrum&, const std::wstring&, portfolio::Portfolio& );
        bool prepare_portfolio( const TXTChromatogram&, const std::wstring&, portfolio::Portfolio& );
    };
}

#endif // DATAFILE_H
