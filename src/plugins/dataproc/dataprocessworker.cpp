/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "dataprocessworker.hpp"
#include "dataprocessor.hpp"
#include "dataprochandler.hpp"
#include "sessionmanager.hpp"
#include "mainwindow.hpp"
#include <qtwrapper/progressbar.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/progressmanager/progressmanager.h>

using namespace dataproc;

DataprocessWorker * DataprocessWorker::instance_ = 0;
std::mutex DataprocessWorker::mutex_;

namespace dataproc { namespace internal {
	class DataprocessWorkerCollector {
	public:
		~DataprocessWorkerCollector() {
			DataprocessWorker::dispose();
		}
	};

	DataprocessWorkerCollector __garbateCollector;
}
}

DataprocessWorker::DataprocessWorker() : work_( io_service_ )
{
}

DataprocessWorker::~DataprocessWorker()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    instance_ = 0;
	io_service_.stop();
    for ( auto& t: threads_ )
        t.join();
}

DataprocessWorker *
DataprocessWorker::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new DataprocessWorker;
    }
    return instance_;
}

void
DataprocessWorker::dispose()
{
	if ( instance_ )
		delete instance_;
}

void
DataprocessWorker::createChromatograms( Dataprocessor *, std::shared_ptr< adcontrols::MassSpectrum >&, double lMass, double hMass )
{
	(void)lMass;
	(void)hMass;
}

void
DataprocessWorker::createChromatograms( Dataprocessor* processor
                                        , const std::vector< std::tuple< int, double, double > >& ranges )
{
	qtwrapper::ProgressBar * p = new qtwrapper::ProgressBar;

    std::lock_guard< std::mutex > lock( mutex_ );
	if ( threads_.empty() )
		threads_.push_back( std::thread( [=] { io_service_.run(); } ) );

	adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
	MainWindow::instance()->getProcessMethod( *pm );

    threads_.push_back( std::thread( [=] { handleCreateChromatograms( processor, pm, ranges, p ); } ) );
}

void
DataprocessWorker::createSpectrogram( Dataprocessor* processor )
{
	qtwrapper::ProgressBar * p = new qtwrapper::ProgressBar;

    std::lock_guard< std::mutex > lock( mutex_ );
	if ( threads_.empty() )
		threads_.push_back( std::thread( [=] { io_service_.run(); } ) );

	adcontrols::ProcessMethodPtr pm = std::make_shared< adcontrols::ProcessMethod >();
	MainWindow::instance()->getProcessMethod( *pm );

    threads_.push_back( std::thread( [=] { handleCreateSpectrogram( processor, pm, p ); } ) );
}

void
DataprocessWorker::join( const std::thread::id& id )
{
    std::lock_guard< std::mutex > lock( mutex_ );

	auto it = std::find_if( threads_.begin(), threads_.end(), [=]( std::thread& t ){ return t.get_id() == id; });
    if ( it != threads_.end() ) {
		it->join();
        threads_.erase( it );
	}
}

void
DataprocessWorker::handleCreateChromatograms( Dataprocessor* processor
                                              , const std::shared_ptr< adcontrols::ProcessMethod > method
                                              , const std::vector< std::tuple< int, double, double > >& ranges
											  , qtwrapper::ProgressBar * progress )
{
    std::vector< adcontrols::Chromatogram > vec;

	progress->setStarted();
    
    if ( const adcontrols::LCMSDataset * dset = processor->getLCMSDataset() ) {
        dset->getChromatograms( ranges, vec, [=](long curr, long total)->bool{
                if ( curr == 0 )
					progress->setProgressRange( 0, total );
				progress->setProgressValue( curr );
				return true;
            } );
    }

    portfolio::Folium folium;
    for ( auto c: vec )
        folium = processor->addChromatogram( c, *method );
	SessionManager::instance()->folderChanged( processor, folium.getParentFolder().name() );

	progress->dispose();

    io_service_.post( std::bind(&DataprocessWorker::join, this, std::this_thread::get_id() ) );
}

void
DataprocessWorker::handleCreateSpectrogram( Dataprocessor* processor
                                            , const std::shared_ptr< adcontrols::ProcessMethod > pm
                                            , qtwrapper::ProgressBar * progress )
{
    progress->setStarted();

    if ( const adcontrols::LCMSDataset * dset = processor->getLCMSDataset() ) {

        bool hasCentroid = dset->hasProcessedSpectrum( 0, 0 );
        const adcontrols::CentroidMethod *centroidMethod = pm->find< adcontrols::CentroidMethod >();

        auto spectra = std::make_shared< adcontrols::MassSpectra >();
        auto objId = dset->findObjId( L"MS.CENTROID" );
        
        adcontrols::Chromatogram tic;
        if ( dset->getTIC( 0, tic ) ) {

            spectra->setChromatogram( tic );

            for ( int pos = 0; pos < int( tic.size() ); ++pos ) {
                if ( auto ptr = std::make_shared< adcontrols::MassSpectrum >() ) {
                    if ( pos == 0 )
                        progress->setProgressRange( 0, static_cast<int>(tic.size()) );
                    progress->setProgressValue( pos );

                    if ( hasCentroid ) {
                        if ( dset->getSpectrum( 0, pos, *ptr, objId ) ) {
                            ADDEBUG() << "Creating spectrogram from centroid: " << pos << "/" << tic.size();
                            (*spectra) << ptr;
                        }
                    } else {
                        adcontrols::MassSpectrum profile;
                        if ( dset->getSpectrum( 0, pos, profile, 0 ) ) {
                            ADDEBUG() << "Creating spectrogram from profile: " << pos << "/" << tic.size();
                            adcontrols::MSPeakInfo result;
                            DataprocHandler::doCentroid( result, *ptr, profile, *centroidMethod );
                            (*spectra) << ptr;
                        }
                    }
                }
            }
        }
        portfolio::Folium folium = processor->addSpectrogram( spectra );
        SessionManager::instance()->folderChanged( processor, folium.getParentFolder().name() );
    }
    
	progress->dispose();

    io_service_.post( std::bind(&DataprocessWorker::join, this, std::this_thread::get_id() ) );
    
}
