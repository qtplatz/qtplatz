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
#include "dataprocessor.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/processmethod.hpp>
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

        impl( adprocessor::dataprocessor * dp )
            : processor_( dp->shared_from_this() ) {
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

    progress( 0, impl_->folio_.size() );

    for ( const auto& folium: impl_->folio_ ) {

        auto peaks = jcb2009_helper::find_peaks().get( folium );
        for ( const auto& peak: peaks ) {
            auto tR = jcb2009_helper::find_peaks().tR( peak );

            if ( auto ms = reader->coaddSpectrum( reader->findPos( std::get< 1 >(tR) )
                                                  , reader->findPos( std::get< 2 >(tR) ) ) ) {

                auto folname = (boost::format( "%s;tR=%.1f(%.1f)" )
                                % folium.name<char>() % std::get<0>(tR) % (std::get<2>(tR) - std::get<1>(tR))).str();

                auto desc = adcontrols::description( { "create", folname } );
                ms->addDescription( desc );

                portfolio::Folium top = impl_->processor_->addSpectrum( ms, adcontrols::ProcessMethod() );

                centroid_processor peak_detector( *impl_->procm_ );
                jcb2009_helper::annotator annotate( folium, *impl_->procm_ );
                auto [pCentroid, pInfo] = peak_detector( *ms );

                if ( pCentroid ) {
                    annotate( pCentroid );
                    pCentroid->addDescription( adcontrols::description( L"process", L"Centroid" ) );
                    top.addAttachment( adcontrols::constants::F_CENTROID_SPECTRUM ).assign( pCentroid, pCentroid->dataClass() );
                }
                if ( pInfo ) {
                    annotate( pInfo );
                    top.addAttachment( adcontrols::constants::F_MSPEAK_INFO ).assign( pInfo, pInfo->dataClass() );
                }
                summary( pCentroid, pInfo );
            }
        }
        ADDEBUG() << " gathering spectra: " << nCurr << "/" << nCount;
        progress(++nCurr, nCount );
    }

    auto [pSummary, pInfoSummary] = summary.get();

    portfolio::Folium sfolium = impl_->processor_->addSpectrum( pSummary, adcontrols::ProcessMethod() );
    sfolium.addAttachment( adcontrols::constants::F_CENTROID_SPECTRUM ).assign( pSummary, pSummary->dataClass() );
    sfolium.addAttachment( adcontrols::constants::F_MSPEAK_INFO ).assign( pInfoSummary, pInfoSummary->dataClass() );

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
