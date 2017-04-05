/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#pragma once

#include <adcontrols/datasubscriber.hpp>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

namespace adcontrols {
    class datafile;
    class CentroidMethod;
    class ChemicalFormula;
    class Chromatogram;
    class LCMSDataset;
    class MassSpectrum;
    class MSPeakInfo;
    class MSLockMethod;
    class ProcessMethod;
    class PeakResult;
    class TargetingMethod;
    class QuanSample;
    class QuanCompounds;
    class QuanResponse;
}

namespace adprocessor {
    class dataprocessor;
}

namespace portfolio { class Portfolio; class Folium;  }
namespace adwidgets { class Progress; }

namespace quan {

    class QuanDataWriter;
    class QuanProcessor;
    class QuanChromatogram;

    ////////////////////
    class FindCompounds;

    ///////////////////////////////////

    class QuanCountingProcessor : public adcontrols::dataSubscriber {
        QuanCountingProcessor( const QuanCountingProcessor& ) = delete;
        QuanCountingProcessor& operator = (const QuanCountingProcessor&) = delete;
    public:
        ~QuanCountingProcessor();

        QuanCountingProcessor( QuanProcessor *, std::vector< adcontrols::QuanSample >& );
        bool operator()( std::shared_ptr< QuanDataWriter > writer );
        QuanProcessor * processor();
        const adcontrols::LCMSDataset * getLCMSDataset() const { return raw_; }
        adcontrols::datafile * datafile() { return datafile_.get(); }
        portfolio::Portfolio * portfolio() { return portfolio_.get(); }
        
    private:
        friend class QuanChromatogramProcessor;

        std::wstring path_;
        const adcontrols::LCMSDataset * raw_;
        std::vector< adcontrols::QuanSample > samples_;
        std::shared_ptr< adcontrols::datafile > datafile_;
        std::shared_ptr< portfolio::Portfolio > portfolio_;
        const std::shared_ptr< adcontrols::ProcessMethod > procmethod_;
        std::shared_ptr< adcontrols::ChemicalFormula > cformula_;
        std::shared_ptr< QuanProcessor > processor_;
        std::shared_ptr< adwidgets::Progress > progress_;
        size_t nFcn_;
        size_t nSpectra_;
        int progress_current_;
        int progress_total_;
        std::mutex mutex_;

        void open();
        bool subscribe( const adcontrols::LCMSDataset& d ) override;
        bool subscribe( const adcontrols::ProcessedDataset& d ) override;
        bool fetch( portfolio::Folium& folium );

        QuanProcessor * quanProcessor();
    public:

    };

}

