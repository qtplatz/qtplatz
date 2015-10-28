/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "resultwriter.hpp"
#include <acqrscontrols/ap240/threshold_result.hpp>
#include <adportable/profile.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>

using namespace ap240;

ResultWriter::ResultWriter()
    : time_datafile_( ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data/ap240_time_data.txt" ).string() )
    , hist_datafile_( ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data/ap240_histogram.txt" ).string() )
{
}


ResultWriter::~ResultWriter()
{
}

ResultWriter&
ResultWriter::operator << ( threshold_result_type& t )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    cache_.push_back( t );
    return *this;
}

void
ResultWriter::commitData() 
{
    std::vector< threshold_result_type > list;

    do {
        std::lock_guard< std::mutex > lock( mutex_ );

        if ( cache_.size() >= 3 ) {
            auto it = std::next( cache_.begin(), cache_.size() - 2 );
            std::move( cache_.begin(), it, std::back_inserter( list ) );
            cache_.erase( cache_.begin(), it );
        }
    } while ( 0 );
    
    if ( !list.empty() ) {
        std::ofstream of( time_datafile_, std::ios_base::out | std::ios_base::app );
        
        if ( !of.fail() ) {
            std::for_each( list.begin(), list.end(), [&of] ( threshold_result_type& a ) {
                    for ( int ch = 0; ch < a.size(); ++ch ) {
                        if ( a[ ch ] ) {
                            of << boost::format( "\nCH-%d, " ) % ch << *a[ ch ];
                        }
                    }
                } );
        }
    }

}

void
ResultWriter::writeHistogram( size_t trigCount
                              , const std::pair< uint64_t, uint64_t >& timeSinceEpoch
                              , std::shared_ptr< adcontrols::MassSpectrum > histogram )
{
    std::ofstream of( hist_datafile_, std::ios_base::out | std::ios_base::app );
        
    const double * times = histogram->getTimeArray();
    const double * counts = histogram->getIntensityArray();
    const auto& prop = histogram->getMSProperty();
        
    of << boost::format( "\n%d, %.8lf, %.14le" )
        % trigCount % ( double( timeSinceEpoch.first ) * 1.0e-9 ) % ( double( timeSinceEpoch.second - timeSinceEpoch.first ) * 1.0e-9 );

    for ( size_t i = 0; i < histogram->size(); ++i )
        of << boost::format( ", %.14le, %d" ) % times[ i ] % uint32_t( counts[ i ] );
}
