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
#include "quanprocessor.hpp"
#include "quanchromatograms.hpp"
#include "quanchromatogramsprocessor.hpp"
#include "quandatawriter.hpp"
#include "quandocument.hpp"
#include "../plugins/dataproc/dataprocconstants.hpp"
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
#include <algorithm>

using namespace quan;

QuanCountingProcessor::~QuanCountingProcessor()
{
}

QuanCountingProcessor::QuanCountingProcessor( QuanProcessor * processor
                                              , std::vector< adcontrols::QuanSample >& samples )
    : raw_( 0 )
    , samples_( samples )
    , procmethod_( processor->procmethod() )
    , cformula_( std::make_shared< adcontrols::ChemicalFormula >() )
    , processor_( processor->shared_from_this() )
    , progress_( adwidgets::ProgressWnd::instance()->addbar() )
    , progress_current_( 0 )
    , progress_total_( 0 )
{
    if ( !samples.empty() )
        path_ = samples[ 0 ].dataSource();
    ADDEBUG() << "CountingProcessor has " << samples.size() << " samples";
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

    double tolerance = 0.001;
    if ( auto tm = procmethod_->find< adcontrols::TargetingMethod >() )
        tolerance = tm->tolerance( adcontrols::idToleranceDaltons ) / 2.0;
    
    adcontrols::CentroidProcess centroidProcess( *cm );

    for ( auto& sample : samples_ ) {

        auto dp = std::make_shared< adprocessor::dataprocessor >();
        std::wstring emsg;
        std::shared_ptr< adcontrols::MassSpectrum > hist;
        adcontrols::MassSpectrum centroid;
        
        if ( dp->open( sample.dataSource(), emsg ) && ( hist = dp->readSpectrumFromTimeCount() ) ) {
            using adcontrols::MSPeakInfo;
            using adcontrols::MSPeakInfoItem;

            std::map< std::string, MSPeakInfoItem > pks;
            
            if ( centroidProcess( *hist ) ) {
                auto pkinfo = centroidProcess.getPeakInfo();
                centroidProcess.getCentroidSpectrum( centroid );
                
                for ( auto& compound: compounds ) {
                    
                    auto beg = std::lower_bound( pkinfo.begin(), pkinfo.end(), compound.mass() - tolerance, [](const auto& a, const double& m){
                            return a.mass() < m;
                        });
                    auto end = std::lower_bound( pkinfo.begin(), pkinfo.end(), compound.mass() + tolerance, [](const auto& a, const double& m){
                            return a.mass() < m;
                        });                        
                    if ( beg != pkinfo.end() ) {
                        
                        auto it = std::max_element( beg, end, [](const auto& a, const auto& b){ return a.area() < b.area(); } );
                        it->formula( compound.formula() ); // assign formula to peak
                        pks[ compound.formula() ] = *it;

                        using adcontrols::annotation;
                        centroid.get_annotations()
                            << annotation( compound.formula()
                                           , it->mass()
                                           , it->area()
                                           , std::distance( pkinfo.begin(), it)
                                           , 1000
                                           , annotation::dataFormula );
                    }
                }

                const double * masses = hist->getMassArray();
                const double * counts = hist->getIntensityArray();
                boost::filesystem::path stem = boost::filesystem::path( sample.dataSource() ).stem();
                
                int n(0);
                for ( const auto& compound: compounds ) {
                    size_t size(0), count(0), idx;
                    auto beg = std::lower_bound( masses, masses + hist->size(), compound.mass() - tolerance );
                    auto end = std::lower_bound( masses, masses + hist->size(), compound.mass() + tolerance );
                    if ( beg != masses + hist->size() ) {
                        idx = std::distance( masses, beg );
                        size = std::distance( beg, end );
                        count = size_t( std::accumulate( counts + idx, counts + idx + size, double(0) ) + 0.5 );
                        
                    }
                    auto pk = pks[ compound.formula() ];
                    
                    adcontrols::QuanResponse resp;
                    resp.uuid_cmpd( compound.uuid() );
                    resp.uuid_cmpd_table( compounds.uuid() );
                    resp.formula( compound.formula() );
                    resp.idx_ = int32_t( idx );
                    resp.fcn_ = 0;
                    resp.mass_ = pk.mass();
                    resp.intensity_ = count;
                    resp.amounts_ = 0;
                    resp.tR_ = 0;

                    if ( auto file = writer->write( *hist, stem.wstring() ) ) {
                        resp.dataGuid_ = file.name();
                        auto att = writer->attach< adcontrols::MassSpectrum >( file, centroid, dataproc::Constants::F_CENTROID_SPECTRUM );
                        writer->attach< adcontrols::ProcessMethod >( att, *procmethod_, L"ProcessMethod" );
                        writer->attach< adcontrols::MSPeakInfo >( file, centroidProcess.getPeakInfo(), dataproc::Constants::F_MSPEAK_INFO );
                        writer->attach< adcontrols::QuanSample >( file, sample, dataproc::Constants::F_QUANSAMPLE );
                    }
                    sample << resp;
                    
                    ADDEBUG() << "\n" << stem.string()
                              << boost::format("\t\"%s\"\t%d\t%d\t%.14lf\t%.14le\t%g" )
                        % compound.formula() % size_t( count + 0.5 ) % size % pk.mass() % pk.time() % pk.area();
                    // of << boost::format("\t\"%s\"\t%d\t%d\t%.14lf\t%.14le\t%g" )
                    //     % compound.formula() % size_t( count + 0.5 ) % size % pk.mass() % pk.time() % pk.area();
                }
            }
        }
        writer->insert_table( sample );
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

bool
QuanCountingProcessor::fetch( portfolio::Folium& folium )
{
    try {
        folium = datafile_->fetch( folium.id(), folium.dataClass() );
        portfolio::Folio attachs = folium.attachments();
        for ( auto att : attachs ) {
            if ( att.empty() )
                fetch( att ); // recursive call make sure for all blongings load up in memory.
        }
    }
    catch ( std::bad_cast& ) {}
    return true;
}

