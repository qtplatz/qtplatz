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

#include "jcb2009_processor.hpp"
#include "jcb2009_helper.hpp"
#include "jcb2009_summarizer.hpp"
#include "centroid_processor.hpp"
#include "generator_property.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/jcb2009_peakresult.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adportfolio/folium.hpp>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <memory>

namespace adprocessor {

    class JCB2009_Processor::impl {
    public:
        std::shared_ptr< adprocessor::dataprocessor > processor_;
        std::shared_ptr< adcontrols::ProcessMethod > procm_;
        std::vector< portfolio::Folium > folio_;
        std::vector< portfolio::Folium > added_;
        std::shared_ptr< const adcontrols::lockmass::mslock > global_lkms_;

        impl( adprocessor::dataprocessor * dp )
            : processor_( dp->shared_from_this() ) {
            global_lkms_ = processor_->dataGlobalMSLock();
        }
    };
}

using namespace adprocessor;

JCB2009_Processor::~JCB2009_Processor()
{
}

JCB2009_Processor::JCB2009_Processor( adprocessor::dataprocessor * processor )
    : impl_( std::make_unique< impl >( processor ) )
{
}

void
JCB2009_Processor::operator << ( portfolio::Folium&& folium )
{
    impl_->folio_.emplace_back( std::move( folium ) );

}

void
JCB2009_Processor::setProcessMethod( std::shared_ptr< adcontrols::ProcessMethod > pm )
{
    impl_->procm_ = std::move( pm );
}

void
JCB2009_Processor::operator()( std::shared_ptr< const adcontrols::DataReader > reader
                               , std::function<bool( size_t, size_t )> progress )
{
    size_t nCount = impl_->folio_.size();
    size_t nCurr = 0;

    jcb2009_helper::summarizer summary;
    std::shared_ptr< adcontrols::MassSpectrum > temp;

    progress( 0, impl_->folio_.size() );

    for ( const auto& cfolium: impl_->folio_ ) {

        jcb2009_helper::find_mass find_mass( cfolium, *impl_->procm_ );

        auto [gen,peaks] = jcb2009_helper::folium_accessor( cfolium )();

        for ( const auto& peak: peaks ) {

            adcontrols::jcb2009_peakresult pkResult( { gen.mass(), gen.mass_width(), gen.protocol() }
                                                     , peak
                                                     , { cfolium.name<char>(), cfolium.uuid() }  );

            int target_protocol = pkResult.protocol();

            auto tR = jcb2009_helper::find_peaks().tR( peak );

            if ( auto ms = reader->coaddSpectrum(
                     reader->findPos( std::get< 1 >(tR) ), reader->findPos( std::get< 2 >(tR) ) ) ) {

                auto folname = (boost::format( "%s;tR=%.1f(%.1f)" )
                                % cfolium.name<char>() % std::get<0>(tR) % (std::get<2>(tR) - std::get<1>(tR))).str();

                ms->addDescription( adcontrols::description( { "create", folname } ) );

                // apply MSLock
                impl_->processor_->mslock( *ms, std::get<0>(tR) );

                auto [pCentroid, pInfo] = centroid_processor( *impl_->procm_ )( *ms );
                if ( pCentroid && pInfo ) {
                    temp = pCentroid;
                    pCentroid->addDescription( adcontrols::description( L"process", L"Centroid" ) );
                    adcontrols::annotation anno;
                    if ( auto idx = find_mass( *pCentroid, target_protocol ) ) {
                        using adcontrols::segments_helper;
                        double mass = segments_helper::get_mass( *pCentroid, *idx );
                        double intensity = segments_helper::get_intensity( *pCentroid, *idx );
                        anno = adcontrols::annotation( (boost::format("AA%.3f@%.1fs") % mass % std::get<0>(tR)).str(), mass, intensity, idx->first );
                        segments_helper::get_annotations( *pCentroid, *idx ) << anno;
                        segments_helper::set_color( *pCentroid, idx->second, idx->first, 15 );
                    } else {
                        // top.setAttribute( "tag", "red" );
                    }
                    // top.addAttachment( adcontrols::constants::F_CENTROID_SPECTRUM ).assign( pCentroid, pCentroid->dataClass() );
                    // top.addAttachment( adcontrols::constants::F_MSPEAK_INFO ).assign( pInfo, pInfo->dataClass() );

                    if ( auto it = find_mass( *pInfo, target_protocol ) ) {
                        pkResult.set_found_mass( **it );
                        summary( **it, std::move( pkResult ) );
                    }
                    // impl_->added_.emplace_back( top );
                }
            }
        }
        progress(++nCurr, nCount );
    }

    auto pSummary = summary.get( *temp );
    auto pInfo = summary.get();

    // auto [pSummary, pInfoSummary] = summary.get();
    portfolio::Folium sfolium = impl_->processor_->addSpectrum( pSummary, *impl_->procm_, true /* update sessionManager */ );

    sfolium.addAttachment( adcontrols::constants::F_CENTROID_SPECTRUM ).assign( pSummary, pSummary->dataClass() );
    sfolium.addAttachment( adcontrols::constants::F_MSPEAK_INFO ).assign( pInfo, pInfo->dataClass() );

    progress(++nCurr, nCount );
    ADDEBUG() << " gathering spectra: " << nCurr << "/" << nCount;
    // todo ---
    // get mass spectrum for peak retention time -- done
    // centroid spectrum, and color code for an ion, which the mass corresponding to a mass of chromatogram extracted.
    // re-constract an ideal mass spectrum that combines all color coded ions;
}

size_t
JCB2009_Processor::num_chromatograms() const
{
    return impl_->folio_.size();
}

const std::vector< portfolio::Folium >
JCB2009_Processor::added() const
{
    return impl_->added_;
}

dataprocessor *
JCB2009_Processor::dataprocessor()
{
    return impl_->processor_.get();
}
