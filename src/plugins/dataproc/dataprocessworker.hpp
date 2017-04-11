/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef DATAPROCESSWORKER_HPP
#define DATAPROCESSWORKER_HPP

#include <workaround/boost/asio.hpp>
#include <adportable/asio/thread.hpp>
#include <mutex>
#include <memory>
#include <thread>
#include <vector>

namespace adcontrols {
    enum hor_axis: unsigned int;
    class MassSpectra;
    class MassSpectrum;
    class ProcessMethod;
    class MSChromatogramMethod;
    class DataReader;
    class MSPeakInfoItem;
    class MSLockMethod;
}

namespace adprot { class digestedPeptides; }
namespace adwidgets { class Progress;  }
namespace boost { namespace uuids { struct uuid; } }

class QString;

namespace dataproc {

	class Dataprocessor;

    class DataprocessWorker {
        DataprocessWorker();
        std::mutex mutex_;
        std::vector< adportable::asio::thread > threads_;
        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
    public:
        ~DataprocessWorker();

        static DataprocessWorker * instance();

        // [0]
        void createChromatogramsByMethod( Dataprocessor *, std::shared_ptr< const adcontrols::ProcessMethod >, const QString& origin );

        // [3] Create from GUI selected m/z|time range
        void createChromatogramByAxisRange( Dataprocessor * processor
                                            , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                            , adcontrols::hor_axis axis
                                             , const std::pair<double, double >& range
                                            , const adcontrols::DataReader * reader );

        // [1]
        void createChromatograms( Dataprocessor * processor
                                  , adcontrols::hor_axis axis
                                  , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges
                                  , const boost::uuids::uuid& dataReaderUuid );
        // [2]
        void createChromatogramsV2( Dataprocessor *, adcontrols::hor_axis, const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& );

        // [4]
        void createChromatogramsV3( Dataprocessor *
                                    , adcontrols::hor_axis
                                    , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges                                    
                                    , const adcontrols::DataReader * reader );

        void createSpectrogram( Dataprocessor * );
		void clusterSpectrogram( Dataprocessor * );
        void findPeptide( Dataprocessor *, const adprot::digestedPeptides& );
        void mslock( Dataprocessor *, std::shared_ptr< adcontrols::MassSpectra >, const adcontrols::MSLockMethod& );
        void exportMatchedMasses( Dataprocessor *, std::shared_ptr< const adcontrols::MassSpectra >, const std::wstring& foliumId );

    private:

        /* Generage chromatogram by MSChromatoramMethod::targets vector */
        // for v2 data format        
        void handleCreateChromatogramsV2( Dataprocessor *
                                          , const adcontrols::MSChromatogramMethod&
                                          , std::shared_ptr< const adcontrols::ProcessMethod >
                                          , std::shared_ptr<adwidgets::Progress> );

        // for v2 data format
        void handleCreateChromatogramsV2( Dataprocessor *
                                          , const std::shared_ptr< adcontrols::ProcessMethod >
                                          , adcontrols::hor_axis
                                          , const std::vector< std::pair< int, adcontrols::MSPeakInfoItem > >& ranges
                                          , std::shared_ptr<adwidgets::Progress> );
        
        // for v3 data format
        void handleChromatogramsByMethod( Dataprocessor *
                                          , const adcontrols::MSChromatogramMethod&
                                          , std::shared_ptr< const adcontrols::ProcessMethod >
                                          , std::shared_ptr< const adcontrols::DataReader >
                                          , int fcn                                        
                                          , std::shared_ptr<adwidgets::Progress> );

        void handleChromatogramByAxisRange( Dataprocessor *
                                            , std::shared_ptr< const adcontrols::ProcessMethod > pm
                                            , adcontrols::hor_axis axis
                                            , const std::pair<double, double >& range
                                            , std::shared_ptr< const adcontrols::DataReader >
                                            , int fcn
                                            , std::shared_ptr<adwidgets::Progress> );
        
        // for v2 data format
        void handleCreateSpectrogram( Dataprocessor *
                                      , const std::shared_ptr< adcontrols::ProcessMethod >
                                      , std::shared_ptr<adwidgets::Progress> );

        // for v3 data format
        void handleCreateSpectrogram( Dataprocessor *
                                      , std::shared_ptr< const adcontrols::ProcessMethod >
                                      , const adcontrols::DataReader *
                                      , int fcn
                                      , std::shared_ptr<adwidgets::Progress> );
        
        void handleClusterSpectrogram( Dataprocessor *
                                       , const std::shared_ptr< adcontrols::ProcessMethod >
                                       , std::shared_ptr<adwidgets::Progress> );

        void handleFindPeptide( Dataprocessor *
                                , const std::shared_ptr< adcontrols::ProcessMethod >
                                , std::shared_ptr<adwidgets::Progress> );
        
        void handleMSLock( Dataprocessor *
                           , std::shared_ptr< adcontrols::MassSpectra >
                           , const adcontrols::MSLockMethod&
                           , std::shared_ptr<adwidgets::Progress> );

        void handleExportMatchedMasses( Dataprocessor *
                                        , std::shared_ptr< const adcontrols::MassSpectra >
                                        , const adcontrols::MSLockMethod&
                                        , std::shared_ptr<adwidgets::Progress> );

        void join( const adportable::asio::thread::id& );
    };

}

#endif // DATAPROCESSWORKER_HPP
