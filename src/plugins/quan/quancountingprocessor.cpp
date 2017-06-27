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

#include "quancountingprocessor.hpp"
#include "findcompounds.hpp"
#include "quanprocessor.hpp"
#include "quanchromatograms.hpp"
#include "quanchromatogramsprocessor.hpp"
#include "quandatawriter.hpp"
#include "quandocument.hpp"
#include "quanprogress.hpp"
#include "../plugins/dataproc/dataprocconstants.hpp"
#include <coreplugin/progressmanager/progressmanager.h>
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanresponse.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/file.hpp>
#include <adfs/cpio.hpp>
#include <adlog/logger.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/debug.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adutils/cpio.hpp>
#include <adwidgets/progresswnd.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <algorithm>

using namespace quan;

QuanCountingProcessor::~QuanCountingProcessor()
{
}

QuanCountingProcessor::QuanCountingProcessor( QuanProcessor * processor
                                              , std::vector< adcontrols::QuanSample >& samples
                                              , std::shared_ptr< ProgressHandler > p )
    : raw_( 0 )
    , samples_( samples )
    , procmethod_( processor->procmethod() )
    , cformula_( std::make_shared< adcontrols::ChemicalFormula >() )
    , processor_( processor->shared_from_this() )
      //, progress_( adwidgets::ProgressWnd::instance()->addbar() )
    , progress_( p )
    , progress_current_( 0 )
    , progress_total_( 0 )
{
    if ( !samples.empty() )
        path_ = samples[ 0 ].dataSource();
    progress_current_ = 0;
    progress_total_ = samples.size();
    // progress_->setRange( int( progress_current_ ), int( progress_total_) );
    (*progress_)( int( progress_current_ ), int( progress_total_) );
}

QuanProcessor *
QuanCountingProcessor::processor()
{
    return processor_.get();
}


bool
QuanCountingProcessor::operator()( std::shared_ptr< QuanDataWriter > writer )
{
    auto cm = procmethod_->find< adcontrols::CentroidMethod >();
    auto qm = procmethod_->find< adcontrols::QuanMethod >();
        
    if ( !cm || !qm )
        return false;

    adcontrols::QuanCompounds compounds;
    if ( auto qc = procmethod_->find< adcontrols::QuanCompounds >() )
        compounds = *qc;

    int channels( 0 ); // 1 := counting channel use, 2 := profile channel use, 3 := both
    for ( const auto& c: compounds )
        channels |= c.isCounting() ? 1 : 2;

    double tolerance = 0.001;
    if ( auto tm = procmethod_->find< adcontrols::TargetingMethod >() )
        tolerance = tm->tolerance( adcontrols::idToleranceDaltons );

    auto lkMethod = procmethod_->find< adcontrols::MSLockMethod >();
    
    for ( auto& sample : samples_ ) {

        const boost::filesystem::path stem = boost::filesystem::path( sample.dataSource() ).stem();
        auto dp = std::make_shared< adprocessor::dataprocessor >();
        std::wstring emsg;

        if ( dp->open( sample.dataSource(), emsg ) ) {

            FindCompounds findCompounds( compounds, *cm, tolerance );
            
            if ( channels & 0x01 ) { // counting 
                if ( auto hist = dp->readSpectrumFromTimeCount() )
                    findCompounds.doCentroid( dp, hist, true );
            }

            if ( channels & 0x02 ) { // profile
                if ( auto profile = dp->readCoAddedSpectrum( false ) )
                    findCompounds.doCentroid( dp, profile, false );
            }
            
            if ( lkMethod && lkMethod->enabled() )
                findCompounds.doMSLock( *lkMethod, true );
            
            if ( channels & 0x01 ) { // counting
                findCompounds( dp, true );
                findCompounds.write( writer, stem.wstring(), procmethod_, sample, true, dp );
            }

            if ( channels & 0x02 ) { // profile
                findCompounds( dp, false );
                findCompounds.write( writer, stem.wstring(), procmethod_, sample, false, dp );
            }
        }
        
        writer->insert_table( sample );
        (*progress_)();
        processor_->complete( &sample );
    }
    QuanDocument::instance()->sample_processed( this );
    return true;
}

bool
QuanCountingProcessor::subscribe( const adcontrols::LCMSDataset& d )
{
    raw_ = &d;
    return true;
}

bool
QuanCountingProcessor::subscribe( const adcontrols::ProcessedDataset& d )
{
    portfolio_ = std::make_shared< portfolio::Portfolio >( d.xml() );
    return true;
}

