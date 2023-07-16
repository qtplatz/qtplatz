// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#pragma once

#include "adprocessor_global.hpp"
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <boost/optional.hpp>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

class QMenu;

namespace adfs { class filesystem; class sqlite; }

namespace adcontrols {
    class CentroidMethod;
    class datafile;
    class DataReader;
    class LCMSDataset;
    class ProcessedDataset;
    class ProcessMethod;
    class MassSpectra;
    class MassSpectrum;
    class MassSpectrometer;
    class MSPeakInfo;
    class MSCalibrationResult;
    namespace lockmass { class mslock; }
}

namespace portfolio { class Portfolio; class Folder; class Folium; }

namespace adprocessor {

    enum ContextID {
        ContextMenuOnProfileMS
        , ContextMenuOnProcessedMS
        , ContextMenuOnMSPeakTable
        , ContextMenuOnNavigator
    };

    class ADPROCESSORSHARED_EXPORT dataprocessor : public std::enable_shared_from_this< dataprocessor >
                                                 , public adcontrols::dataSubscriber {

    public:
        virtual ~dataprocessor();
        dataprocessor();

        // dataprocessor
        virtual void setMode( int id );  // main
        virtual int mode() const;

        virtual void setModified( bool );

        virtual bool open( const std::filesystem::path&, std::string& errmsg );

		virtual std::wstring filename() const;

        virtual void setFile( std::unique_ptr< adcontrols::datafile >&& );
        virtual adcontrols::datafile * file();
        virtual const adcontrols::datafile * file() const;

        virtual const adcontrols::LCMSDataset * rawdata();

        std::shared_ptr< adfs::sqlite > db() const override;

        virtual const portfolio::Portfolio& portfolio() const;
        virtual portfolio::Portfolio& portfolio();

        virtual std::shared_ptr< adcontrols::MassSpectrum > readSpectrumFromTimeCount();
        virtual std::shared_ptr< adcontrols::MassSpectrum > readCoAddedSpectrum( bool histogram = false, int proto = (-1) );

        virtual std::shared_ptr< adcontrols::MassSpectrometer > massSpectrometer();

        static bool doCentroid( adcontrols::MSPeakInfo& pkInfo
                                , adcontrols::MassSpectrum& centroid
                                , const adcontrols::MassSpectrum& profile
                                , const adcontrols::CentroidMethod& m );

        static
        boost::optional< std::pair< std::shared_ptr< adcontrols::MSPeakInfo >
                                    , std::shared_ptr< adcontrols::MassSpectrum > > >
            doCentroid( const adcontrols::MassSpectrum& profile
                        , const adcontrols::ProcessMethod& procm );

        virtual uint64_t countTimeCounts( const adcontrols::MassSpectrum&, double lMass, double uMass );

        // implement adcontrols::dataSubscriber
        virtual bool subscribe( const adcontrols::LCMSDataset& ) override;
        virtual bool subscribe( const adcontrols::ProcessedDataset& ) override;
        virtual void notify( adcontrols::dataSubscriber::idError, const std::string& ) override;
        // <----
        virtual void addContextMenu( ContextID, QMenu&, std::shared_ptr< const adcontrols::MassSpectrum >, const std::pair< double, double >&, bool isTime );
        virtual void addContextMenu( ContextID, QMenu&, const portfolio::Folium& );
        virtual bool estimateScanLaw( std::shared_ptr< const adcontrols::MassSpectrum >, const std::vector< std::pair<int, int> >& );

        virtual bool export_text( const portfolio::Folium&, std::ostream& ) const;

        adfs::filesystem * fs();
        const adfs::filesystem * fs() const;

        std::shared_ptr< adcontrols::MassSpectra >
            createSpectrogram( std::shared_ptr< const adcontrols::ProcessMethod > pm
                               , const adcontrols::DataReader * reader
                               , int proto
                               , std::function< bool(size_t, size_t) > progress ) const;

        // mass peaks selected by chromatograms
        virtual void xicSelectedMassPeaks( adcontrols::MSPeakInfo&& info );
        virtual void markupMassesFromChromatograms( portfolio::Folium&& folium ) {};

        bool applyCalibration( const adcontrols::MSCalibrateResult& );
        //-------------->

        std::shared_ptr< const adcontrols::lockmass::mslock > dataGlobalMSLock() const;
        void handleGlobalMSLockChanged();

        //---
        // bool apply_mslock( std::shared_ptr< adcontrols::MassSpectrum > ) const; // apply mass lock if data global lockmass exists
        static bool mslock( adcontrols::MassSpectrum&, const adcontrols::lockmass::mslock& );

        bool fetch( portfolio::Folium& );

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

} // mpxcontrols
