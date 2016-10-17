/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "acqrsdata.hpp"
#include "resultwriter.hpp"
#include <acqrscontrols/find_threshold_peaks.hpp>
#include <acqrscontrols/find_threshold_timepoints.hpp>
#include <acqrscontrols/ap240/tdcdoc.hpp>
#include <acqrscontrols/ap240/threshold_result.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <adfs/sqlite.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adportable/counting/counting_result.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/debug.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/format.hpp>
#include <iostream>

static const boost::uuids::uuid ap240_observer = boost::uuids::string_generator()( "{76d1f823-2680-5da7-89f2-4d2d956149bd}" );

acqrsdata::acqrsdata() : polarity_( positive_polarity )
                       , threshold_( 0.010 )
                       , processor_( std::make_shared< adprocessor::dataprocessor >() )
{
    adplugin::manager::standalone_initialize(); 
}

bool
acqrsdata::open( const boost::filesystem::path& path )
{
    path_ = path;
    std::wstring msg;
    if ( processor_->open( path.wstring(), msg ) ) {
        if ( auto file = processor_->rawdata() ) {
            if ( file->dataformat_version() == 3 )
                return true;
        }
    }
    return false;
}

void
acqrsdata::setThreshold( double t )
{
    threshold_ = t;
}

void
acqrsdata::setPolairty( enum polarity t )
{
    polarity_ = t;
}


enum acqrsdata::polarity
acqrsdata::polarity() const
{
    return polarity_;
}

bool
acqrsdata::processIt( std::function< void( size_t, size_t ) > progress )
{
    using adcontrols::threshold_method;
    threshold_method method;

    method.enable          = true;
    method.threshold_level = threshold_;
    method.time_resolution = 0;
    method.response_time   = 0;
    method.use_filter      = false;
    method.slope           = polarity_ == positive_polarity ? threshold_method::CrossUp : threshold_method::CrossDown;
    method.algo_           = threshold_method::Absolute;
    
    if ( auto dp = processor_ ) {

        ResultWriter writer( *dp->db() );

        adfs::stmt sql( *dp->db() );
        
        size_t size(0);
        size_t idx(0);
        sql.prepare( "SELECT count(*) FROM AcquiredData WHERE objuuid = '76d1f823-2680-5da7-89f2-4d2d956149bd'" );
        if ( sql.step() == adfs::sqlite_row )
            size = sql.get_column_value< uint64_t >( 0 );

        if ( auto reader = dp->rawdata()->dataReader( ap240_observer ) ) {

            sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid = '76d1f823-2680-5da7-89f2-4d2d956149bd'" );
	  
            while( sql.step() == adfs::sqlite_row ) {
                
                auto rowid = sql.get_column_value< int64_t >( 0 );
                boost::any a = reader->getData( rowid ); // ( it->rowid() );
                if ( a.type() == typeid( std::shared_ptr< acqrscontrols::ap240::waveform > ) ) {
                    if ( auto ptr = boost::any_cast< std::shared_ptr< acqrscontrols::ap240::waveform > >( a ) ) {
                        if ( auto rp = processThreshold3( ptr, method ) ) {

                            writer << rp;
                            auto wp = rp->data(); // waveform
                            
                            if ( ( idx++ % 100 ) == 0 )
                                progress( idx, size );
                            
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}

std::shared_ptr< acqrscontrols::ap240::threshold_result >
acqrsdata::processThreshold3( std::shared_ptr< const acqrscontrols::ap240::waveform > waveform
                            , const adcontrols::threshold_method& method )
{
    auto result = std::make_shared< acqrscontrols::ap240::threshold_result >( waveform );
    
    //result->setFindUp( method.slope == adcontrols::threshold_method::CrossUp );
    result->threshold_level() = method.threshold_level;
    result->algo() = static_cast< enum adportable::counting::counting_result::algo >( method.algo_ );

    adcontrols::CountingMethod range;
    range.setEnable( false );
            
    const auto idx = waveform->method_.protocolIndex();
    
    if ( method.enable ) {
        
        if ( method.algo_ == adcontrols::threshold_method::Differential ) {
            if  ( method.slope == adcontrols::threshold_method::CrossUp ) {
                acqrscontrols::find_threshold_peaks< true, acqrscontrols::ap240::waveform > find_peaks( method, range );
                find_peaks( *waveform, *result, result->processed() );
            } else {
                acqrscontrols::find_threshold_peaks< false, acqrscontrols::ap240::waveform > find_peaks( method, range );
                find_peaks( *waveform, *result, result->processed() );
            }
        } else {
            acqrscontrols::find_threshold_timepoints< acqrscontrols::ap240::waveform > find_threshold( method, range );
            find_threshold( *waveform, *result, result->processed() );
        }
        
    }

    return result;
}
