/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "extract_by_generator_property.hpp"
#include "dataprocessor.hpp"
#include "generator_property.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <regex>


using namespace adprocessor::chromatogr_extractor;

extract_by_generator_property::~extract_by_generator_property()
{
}

extract_by_generator_property::extract_by_generator_property( const adcontrols::LCMSDataset * file, dataprocessor * dp )
    : extractor_( std::make_unique< v3::MSChromatogramExtractor >( file, dp ) )
{
}

std::vector< std::shared_ptr< adcontrols::Chromatogram > >
extract_by_generator_property::operator()( const adcontrols::ProcessMethod& pm
                                           , std::shared_ptr< const adcontrols::DataReader > reader
                                           , const std::vector< generator_property >& properties
                                           , std::function<bool( size_t, size_t )> progress )
{
    double width = 0;
    if ( const adcontrols::MSChromatogramMethod * cm = pm.find< adcontrols::MSChromatogramMethod >() )
        width = cm->width( cm->widthMethod() );

    const size_t nCounts = reader->size( -1 ) * 2;
    size_t nProg(0);

    const bool isCounting = std::regex_search( reader->objtext()
                                               , std::regex( "^histogram.*$|^pkd\\.[1-9]\\.u5303a\\.ms-cheminfo.com" ) );

    vec_ = {};
    for ( const auto& prop: properties ) {
        vec_.emplace_back( 0, 0, std::make_shared< adcontrols::Chromatogram >() );
        auto& t = vec_.back();
        auto chro = std::get< 2 >( t );
        chro->addDescription( adcontrols::description(
                                  {"create"
                                   , (boost::format( "m/z %.3lf(W %.1fmDa),%s,p%d" )
                                      % prop.mass()
                                      % (prop.mass_width() * 1000) // to mDa
                                      % reader->abbreviated_display_name()
                                      % prop.protocol()).str() } ) );
        if ( auto desc = extractor_->mslock_description() )
            chro->addDescription( *desc );
        chro->setIsCounting( isCounting );
        chro->setGeneratorProperty( boost::json::serialize( boost::json::value_from( prop ) ) );
        // ADDEBUG() << "add chromatogram: " << adcontrols::Chromatogram::make_folder_name<char>( chro->descriptions() );
    }

    //////////////////////////////////////////////////////////////////////////////

    if ( extractor_->loadSpectra( &pm, reader, -1, progress, nCounts, nProg ) ) {

        for ( auto& spectrum : extractor_->spectra() ) {
            auto [pos,ms] = spectrum;
            double time = ms->getMSProperty().timeSinceInjection();

            auto it = vec_.begin();
            for ( const auto& prop: properties ) {
                auto pChr = std::get< 2 >( *it );
                auto mass_range = std::make_pair( prop.mass() - prop.mass_width() / 2.0
                                                  , prop.mass() + prop.mass_width() / 2.0 );
                if ( auto y = extractor_->computeIntensity( *ms, adcontrols::hor_axis_mass, mass_range ) ) {
                    if ( std::get< 0 >( *it )++ == 0 ) // count++
                        std::get< 1 >( *it ) = *y;
                    else
                        (*pChr) << std::make_pair( time, *y - std::get< 1 >( *it ) );
                } else {
                    ADDEBUG() << "intensity not returned at time: " << time;
                }
                ++it;
            }
            progress( ++nProg, nCounts );
        }

        std::pair< double, double > time_range =
            std::make_pair( extractor_->spectra().begin()->second->getMSProperty().timeSinceInjection()
                            , extractor_->spectra().rbegin()->second->getMSProperty().timeSinceInjection() );

        std::for_each( vec_.begin(), vec_.end()
                       , [&]( auto t ){
                           auto chro = std::get< 2 >( t );
                           chro->setMinimumTime( std::get< 0 >(time_range) );
                           chro->setMaximumTime( std::get< 1 >(time_range) );
                           chro->setAxisLabel( adcontrols::plot::yAxis, isCounting ? "Counts" : "Intensity" );
                           chro->setAxisUnit( isCounting ? adcontrols::plot::Counts : adcontrols::plot::Arbitrary );
                       });
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > retv;
        std::transform( vec_.begin(), vec_.end(), std::back_inserter( retv )
                        , []( auto& t ){
                            return std::move( std::get< 2 >( t ) );
                        });

        return retv;
    }
    return {};

}
