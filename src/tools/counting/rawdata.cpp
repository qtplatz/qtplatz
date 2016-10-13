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

#include "rawdata.hpp"
#include <adfs/sqlite.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adplugin_manager/manager.hpp>
#include <adportable/debug.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <acqrscontrols/ap240/tdcdoc.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/format.hpp>
#include <iostream>

static const boost::uuids::uuid ap240_observer = boost::uuids::string_generator()( "{76d1f823-2680-5da7-89f2-4d2d956149bd}" );

rawdata::rawdata() : polarity_( negative_polarity )
                   , threshold_( -0.010 )
                   , processor_( std::make_shared< adprocessor::dataprocessor >() )
{
    adplugin::manager::standalone_initialize(); 
}

bool
rawdata::open( const boost::filesystem::path& path )
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
rawdata::setThreshold( double t )
{
    threshold_ = t;
}

void
rawdata::setPolairty( enum polarity t )
{
    polarity_ = t;
}


enum rawdata::polarity
rawdata::polarity() const
{
    return polarity_;
}

bool
rawdata::processIt( std::function< void( size_t, size_t ) > progress )
{
    if ( auto dp = processor_ ) {

        adfs::stmt sql( *dp->db() );
        
        size_t size(0);
        sql.prepare( "SELECT count(*) FROM AcquiredData WHERE objuuid = '76d1f823-2680-5da7-89f2-4d2d956149bd'" );
        if ( sql.step() == adfs::sqlite_row )
            size = sql.get_column_value< uint64_t >( 0 );
        
        sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid = '76d1f823-2680-5da7-89f2-4d2d956149bd'" );        
        
        if ( auto reader = dp->rawdata()->dataReader( ap240_observer ) ) {
	  
            // for ( auto it = reader->begin(); it != reader->end(); ++it ) {
            while( sql.step() == adfs::sqlite_row ) {
                auto rowid = sql.get_column_value< int64_t >( 0 );
                boost::any a = reader->getData( rowid ); // ( it->rowid() );
                if ( a.type() == typeid( std::shared_ptr< acqrscontrols::ap240::waveform > ) ) {
                    if ( auto ptr = boost::any_cast< std::shared_ptr< acqrscontrols::ap240::waveform > >( a ) ) {
                        tdc( ptr );
                    }
                }
            }
            return true;
        }
    }
    return false;
}

void
rawdata::tdc( std::shared_ptr< acqrscontrols::ap240::waveform > ptr )
{
    adcontrols::threshold_method method;
    method.enable = true;
    method.threshold_level = threshold_;
    method.time_resolution = 0;
    method.response_time   = 0;
    method.use_filter      = false;
    method.algo_           = adcontrols::threshold_method::Absolute;
    
    std::vector< uint32_t > elements;
    std::vector< double > processed;

    acqrscontrols::ap240::tdcdoc::find_threshold_timepoints( *ptr, method, elements, processed );

    ADDEBUG() << boost::format( "s/n=%d dtyp=%d size=%d SF=%g, offs=%g, threshold: %d" )
        % ptr->serialnumber_ % ptr->meta_.dataType % ptr->size() % ptr->meta_.scaleFactor % ptr->meta_.scaleOffset % elements.size() ;
    
    // if ( ptr->meta_.dataType == 1 ) {
    //     const int8_t * data = ptr->data< int8_t >();
    //     for ( int i = 0; i < ptr->size() && i < 16; ++i )
    //         std::cout << int( data[i] ) << ", ";
    //     std::cout << std::endl;
    // }
    
}
