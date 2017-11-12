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
#include <adfs/sqlite.hpp>
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

    progress( 0, nSpectra * 2 );
    size_t n( 0 );
    
    auto cm = pm->find< adcontrols::CentroidMethod >();
    if ( !cm )
        return false;
    
    auto lockm = pm->find< const adcontrols::MSLockMethod >();
    if ( !lockm )
        return false;

    if ( lockm->molecules().empty() )
        return false;

    adcontrols::MSFinder finder( lockm->tolerance( lockm->toleranceMethod() ), lockm->algorithm(), lockm->toleranceMethod() );

    auto sql = adfs::stmt( *dp->db() );
    sql.exec( "CREATE TABLE IF NOT EXISTS MassReference ("
              " id INTEGER PRIMARY KEY"
              ", formula TEXT"
              ", exactMass REAL"
              ")"
        );

    sql.exec( "CREATE TABLE IF NOT EXISTS ReferenceTof ("
              " rowid INTEGAR"
              ", refid INTEGER"
              ", protocol INTEGER"
              ", mode INTEGER"
              ", mass REAL"
              ", time REAL"
              ", UNIQUE(rowid,refid)"
              ")"
        );

    sql.exec( "DELETE FROM MassReference" );
    sql.exec( "DELETE FROM ReferenceTof" );
    
    sql.prepare( "INSERT OR REPLACE INTO MassReference ( id, formula, exactMass ) VALUES ( ?,?,? )" );
    int molId(0);
    for ( auto& mol: lockm->molecules().data() ) {
        if ( mol.enable() ) {
            sql.bind( 1 ) = molId++;
            sql.bind( 2 ) = std::string( mol.formula() );
            sql.bind( 3 ) = mol.mass();
            sql.step();
            sql.reset();
        }
    }

    typedef std::tuple< int64_t, int, int, int, double, double > record_type;
    std::vector< record_type > results;

    for ( auto it = reader->begin( proto ); it != reader->end(); ++it ) {
        
        auto ms = reader->getSpectrum( it->rowid() );
        // auto ms = reader->readSpectrum( it );
        adcontrols::MassSpectrum centroid;

        doCentroid( centroid, *ms, *cm );

        int mode = ms->getMSProperty().mode();  // a.k.a. nlaps
        int protocolId = ms->protocolId();

        molId = 0;
        for ( auto& mol: lockm->molecules().data() ) {
            if ( mol.enable() ) {
                size_t idx = finder( centroid, mol.mass() );
                if ( idx != adcontrols::MSFinder::npos ) {
                    results.emplace_back( it->rowid()
                                          , molId
                                          , protocolId
                                          , mode
                                          , centroid.getMass( idx )
                                          , centroid.getTime( idx ) );
                }
                ++molId;
            }
        }
        
        if ( progress( ++n, nSpectra ) )
            return false;
    }

    std::sort( results.begin(), results.end(), [](const auto& a, const auto& b){ return std::get<0>(a) < std::get<0>(b); } );

    sql.prepare( "INSERT INTO ReferenceTof (rowid,refid,protocol,mode,mass,time) VALUES (?,?,?,?,?,?)" );
    
    for ( auto& rec: results ) {

        sql.reset();
        sql.bind( 1 ) = std::get< 0 >( rec ); // rowid
        sql.bind( 2 ) = std::get< 1 >( rec ); // refid := modId
        sql.bind( 3 ) = std::get< 2 >( rec ); // protocolId
        sql.bind( 4 ) = std::get< 3 >( rec ); // mode := nlaps
        sql.bind( 5 ) = std::get< 4 >( rec ); // mass (observed)
        sql.bind( 6 ) = std::get< 5 >( rec ); // time

#if 0
        ADDEBUG() << "Ref rowid(" << std::get< 0 >( rec )
                  << ")[" << std::get< 1 >( rec )
                  << "] protocol=" << std::get< 2 >( rec )
                  << ", mass=" << std::get< 3 >( rec )
                  << ", time=" << std::get< 4 >( rec ) * 1e6;
#endif

        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "sql error";

        if ( progress( ++n, nSpectra ) )
            break;
    }
    // SELECT min(rowid),* FROM ReferenceTof WHERE rowid > ? GROUP BY protocol
    
    
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
    
    if ( raw == nullptr || raw->dataformat_version() <= 2 )
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

