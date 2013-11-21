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
#include <adfs/adfs.hpp>
#include <memory>

namespace adcontrols {
    class Chromatogram;
    class MassSpectrum;
    class ProcessedDataset;
	class MSCalibrateResult;
}

namespace portfolio { class Portfolio; }

namespace addatafile {

    class rawdata;

    class datafile : public adcontrols::datafile {
    public:
        virtual ~datafile();
        datafile();
        
        bool open( const std::wstring& filename, bool readonly = false );

        //--------- implement adcontrols::datafile ----------------
        void accept( adcontrols::dataSubscriber& ) override;
        boost::any fetch( const std::wstring& path, const std::wstring& dataType ) const override;

        // create, modify and delete methods
        bool saveContents( const std::wstring&, const portfolio::Portfolio&, const adcontrols::datafile& ) override;
        bool saveContents( const std::wstring&, const portfolio::Portfolio& ) override;
        bool loadContents( const std::wstring& path, const std::wstring& id, adcontrols::dataSubscriber& ) override;

        bool applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& ) override;
        bool readCalibration( size_t idx, adcontrols::MSCalibrateResult& ) const override;

        adcontrols::datafile::factory_type factory() override { return 0; }

    private:
        bool loadContents( portfolio::Portfolio&, const std::wstring& query );

    private:
        bool mounted_;
        std::wstring filename_;
        adfs::filesystem dbf_;
		std::unique_ptr< adcontrols::ProcessedDataset > processedDataset_;
        std::unique_ptr< rawdata > rawdata_;
        std::map< int, std::shared_ptr< adcontrols::MSCalibrateResult > > calibrations_; // mode, calibration pair
        bool calibration_modified_;
    };

}

#endif // DATAFILE_H
