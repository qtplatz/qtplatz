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

#include "scanlawextractor.hpp" // v3 only supported
#include "xchromatogram.hpp"
#include "centroidmethod.hpp"
#include "centroidprocess.hpp"
#include "chemicalformula.hpp"
#include "chromatogram.hpp"
#include "constants.hpp"
#include "dataprocessor.hpp"
#include "description.hpp"
#include "descriptions.hpp"
#include "lcmsdataset.hpp"
#include "lockmass.hpp"
#include "massspectrum.hpp"
#include "moltable.hpp"
#include "mschromatogrammethod.hpp"
#include "msfinder.hpp"
#include "mslockmethod.hpp"
#include "msproperty.hpp"
#include "processmethod.hpp"
#include "waveform_filter.hpp"
#include <adcontrols/constants.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adportable/debug.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/unique_ptr.hpp>
#include <adportable/utf.hpp>
#include <adutils/acquiredconf.hpp>
#include <boost/format.hpp>
#include <numeric>
#include <ratio>
#include <set>

using namespace adprocessor::v3;

namespace adprocessor {
    namespace v3 {
        bool
        doMSLock( adcontrols::lockmass::mslock& mslock
                  , const adcontrols::MassSpectrum& centroid
                  , const adcontrols::MSLockMethod& m )
        {
#if 0
            // TODO: consider how to handle segmented spectrum -- current impl is always process first 
            adcontrols::MSFinder find( m.tolerance( m.toleranceMethod() ), m.algorithm(), m.toleranceMethod() );
        
            for ( auto& msref : msrefs_ ) {
                size_t idx = find( centroid, msref.second );
                if ( idx != adcontrols::MSFinder::npos ) 
                    mslock << adcontrols::lockmass::reference( msref.first, msref.second, centroid.getMass( idx ), centroid.getTime( idx ) );
            }
        
            if ( mslock.fit() ) {
                // mslock( centroid, true );
                return true;
            }
#endif
            return false;
        }
    }
}


ScanLawExtractor::~ScanLawExtractor()
{
}

ScanLawExtractor::ScanLawExtractor()
{
}


bool
ScanLawExtractor::loadSpectra( std::shared_ptr< adprocessor::dataprocessor > dp
                               , const adcontrols::ProcessMethod * pm
                               , std::shared_ptr< const adcontrols::DataReader > reader
                               , int proto
                               , std::function<bool( size_t, size_t )> progress )
{
    // auto it = reader->begin( proto ); 
    size_t nSpectra = reader->size( proto );
    
    if ( nSpectra == 0 )
        return false;

    auto cm = pm->find< adcontrols::CentroidMethod >();
    if ( !cm )
        return false;
    
    progress( 0, nSpectra );
    
    size_t n( 0 );
    
    for ( auto it = reader->begin( proto ); it != reader->end(); ++it ) {
        
        auto ms = reader->getSpectrum( it->rowid() );
        // auto ms = reader->readSpectrum( it );
        adcontrols::MassSpectrum centroid;

        doCentroid( centroid, *ms, *cm );

        auto& prop = ms->getMSProperty();
        int proto = ms->protocolId();
        int nproto = ms->nProtocols();
        auto range = std::make_pair( ms->getMass( 0 ), ms->getMass( ms->size() - 1 ) );
        
        ADDEBUG() << n << ")" << prop.timeSinceInjection() << " proto=" << proto << "/" << nproto
                  << " m/z(" << range.first << ", " << range.second << ") lap=" << prop.mode();
        
        // if ( doLock )
        //     impl_->apply_mslock( ms, *pm, mslock );
        
        //impl_->spectra_[ it->pos() ] = ms; // (:= pos sort order) keep mass locked spectral series
        
        if ( progress( ++n, nSpectra ) )
            return false;
    }
    return true; //! impl_->spectra_.empty();
}

///////////////////////////////////////////////////////////////////
////// [0] Create chromatograms by a list of molecules    /////////
///////////////////////////////////////////////////////////////////
bool
ScanLawExtractor::operator()( std::shared_ptr< adprocessor::dataprocessor > dp
                              , const adcontrols::ProcessMethod& pm
                              , const std::string& objtext
                              , int proto
                              , std::function<bool( size_t, size_t )> progress )
{
    auto raw = dp->rawdata();
    
    if ( raw->dataformat_version() <= 2 )
        return false;

    std::shared_ptr< const adcontrols::DataReader > reader;

    for ( auto& t: raw->dataReaders() ) {
        if ( t->objtext() == objtext )
            reader = t;
    }
    
    if ( ! reader )
        return false;

    if ( loadSpectra( dp, &pm, reader, -1, progress ) )
        return true;

    return false;
}

bool
ScanLawExtractor::doCentroid(adcontrols::MassSpectrum& centroid
                             , const adcontrols::MassSpectrum& profile
                             , const adcontrols::CentroidMethod& m )
{
    adcontrols::CentroidProcess peak_detector;
    bool result = false;
    
    centroid.clone( profile, false );
    
    if ( peak_detector( m, profile ) )
        result = peak_detector.getCentroidSpectrum( centroid );
    
    if ( profile.numSegments() > 0 ) {
        for ( size_t fcn = 0; fcn < profile.numSegments(); ++fcn ) {
            auto temp = std::make_shared< adcontrols::MassSpectrum >();
            result |= peak_detector( profile.getSegment( fcn ) );
            // pkInfo.addSegment( peak_detector.getPeakInfo() );
            peak_detector.getCentroidSpectrum( *temp );
            centroid <<  std::move( temp );
        }
    }
    return result;
}

