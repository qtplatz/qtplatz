/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANSAMPLEPROCESSOR_HPP
#define QUANSAMPLEPROCESSOR_HPP

#include <adcontrols/datasubscriber.hpp>
#include <string>
#include <vector>
#include <memory>

namespace adcontrols {
    class datafile;
    class QuanSample;
    class QuanCompounds;
    class LCMSDataset;
    class ProcessMethod;
    class MassSpectrum;
    class MSPeakInfo;
    class CentroidMethod;
    class MSLockMethod;
    class TargetingMethod;
    class ChemicalFormula;
}

namespace portfolio { class Portfolio; class Folium;  }


namespace quan {

    class QuanDataWriter;
    class QuanProcessor;

    class QuanSampleProcessor : public adcontrols::dataSubscriber {
        QuanSampleProcessor( const QuanSampleProcessor& ) = delete;
        QuanSampleProcessor& operator = (const QuanSampleProcessor&) = delete;
    public:
        ~QuanSampleProcessor();

        QuanSampleProcessor( QuanProcessor *, std::vector< adcontrols::QuanSample >& );
        bool operator()( std::shared_ptr< QuanDataWriter > writer );
        
    private:
        std::wstring path_;
        const adcontrols::LCMSDataset * raw_;
        std::vector< adcontrols::QuanSample > samples_;
        std::shared_ptr< adcontrols::datafile > datafile_;
        std::shared_ptr< portfolio::Portfolio > portfolio_;
        std::shared_ptr< adcontrols::ChemicalFormula > cformula_;
        const std::shared_ptr< adcontrols::ProcessMethod > procmethod_;

        void open();
        bool subscribe( const adcontrols::LCMSDataset& d ) override;
        bool subscribe( const adcontrols::ProcessedDataset& d ) override;
        bool fetch( portfolio::Folium& folium );

        bool generate_spectrum( const adcontrols::LCMSDataset *, const adcontrols::QuanSample&, adcontrols::MassSpectrum& );
        size_t read_first_spectrum( const adcontrols::LCMSDataset *, adcontrols::MassSpectrum&, uint32_t tidx /* tic index */);
        size_t read_next_spectrum( size_t pos, const adcontrols::LCMSDataset *, adcontrols::MassSpectrum& );

        void processIt( adcontrols::QuanSample&, adcontrols::MassSpectrum& ms, QuanDataWriter * writer );

        static bool doCentroid( adcontrols::MSPeakInfo& pkInfo
                                , adcontrols::MassSpectrum& res
                                , const adcontrols::MassSpectrum& profile
                                , const adcontrols::CentroidMethod& m );

        bool doMSLock( adcontrols::MSPeakInfo& pkInfo // will override
                              , adcontrols::MassSpectrum& centroid // will override
                              , const adcontrols::MSLockMethod& m );

        bool doMSFind( adcontrols::MSPeakInfo& pkInfo
                       , adcontrols::MassSpectrum& res
                       , adcontrols::QuanSample&
                       , const adcontrols::QuanCompounds& 
                       , const adcontrols::TargetingMethod& m );

    };

}

#endif // QUANSAMPLEPROCESSOR_HPP
