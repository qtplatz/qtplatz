
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

#include "jcb2009_summarizer.hpp"
#include "jcb2009_helper.hpp"
#include "jcb2009_peakresult.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adportable/debug.hpp>
#include <boost/json.hpp>
#include <boost/format.hpp>
#include <map>

namespace adprocessor {
    namespace jcb2009_helper {

        class summarizer::impl {
        public:
            impl() {
            }
            std::shared_ptr< adcontrols::MassSpectrum > pSummary_;
            std::shared_ptr< adcontrols::MSPeakInfo > pInfo_;
            std::vector< adcontrols::jcb2009_peakresult > pkResults_;
            std::map< int, std::vector< adcontrols::MSPeakInfoItem > > pInfo;

            adcontrols::MSPeakInfo::iterator
            operator << ( adcontrols::MSPeakInfoItem&& t ) {
                if ( ! pInfo_ )
                    pInfo_ = std::make_shared< adcontrols::MSPeakInfo >();
                auto it = std::upper_bound( pInfo_->begin(), pInfo_->end(), t
                                            , [](const auto& a, const auto& b){ return a.mass() < b.mass(); } );
                return pInfo_->emplace( it, std::move( t ) );
            }

            std::vector< adcontrols::jcb2009_peakresult >::iterator
            operator << ( adcontrols::jcb2009_peakresult&& t ) {
                auto it = std::upper_bound( pkResults_.begin(), pkResults_.end(), t );
                return pkResults_.emplace( it, std::move( t ) );
            }
        };
    }
}

using namespace adprocessor::jcb2009_helper;

summarizer::~summarizer()
{
    delete impl_;
}

summarizer::summarizer() : impl_( new impl() )
{
}

void
summarizer::operator()( const adcontrols::MSPeakInfoItem& item
                        , adcontrols::jcb2009_peakresult&& pkResult )
{
    (*impl_) << adcontrols::MSPeakInfoItem{ item };
    (*impl_) << std::move( pkResult );
}


void
summarizer::operator()( std::shared_ptr< const adcontrols::MassSpectrum > pCentroid )
{
    typedef adcontrols::MassSpectrum T;

    if ( !impl_->pSummary_ ) {
        impl_->pSummary_ = std::make_shared< T >( *pCentroid );
        impl_->pSummary_->addDescription( adcontrols::description( L"create", L"Summary" ) );

        for ( auto& ms: adcontrols::segment_wrapper< T >( *impl_->pSummary_ ) )
            ms.resize( 0 );
    }

    auto dIt = adcontrols::segment_wrapper< T >( *impl_->pSummary_ ).begin();

    for ( auto& ms: adcontrols::segment_wrapper< const T >( *pCentroid ) ) {
        for ( auto anno: ms.annotations() ) {
            if ( anno.index() >= 0 ) {
                size_t idx = (*dIt) << std::make_pair( ms.mass( anno.index() ), ms.intensity( anno.index() ) );
                anno.index( int(idx) ); // replace index
                (*dIt).addAnnotation( std::move( anno ) );
            }
        }
        ++dIt;
    }
}

std::shared_ptr< adcontrols::MassSpectrum >
summarizer::get( const adcontrols::MassSpectrum& t ) const
{
    auto ms = std::make_shared< adcontrols::MassSpectrum >();
    ms->clone( t, false );

    adcontrols::annotations annots;
    std::vector< double > masses, intens;
    for ( auto it = impl_->pkResults_.begin(); it != impl_->pkResults_.end(); ++it ) {
        masses.emplace_back( it->matched_mass() );
        intens.emplace_back( it->matched_mass_height() );
        auto idx = std::distance( impl_->pkResults_.begin(), it );

        annots << adcontrols::annotation{ boost::json::value_from( *it )
                , it->matched_mass()
                , it->matched_mass_height()
                , int( idx )
                , int( it->matched_mass_height() )
                , adcontrols::annotation::flag_jcb2009 };

        annots << adcontrols::annotation{ ( boost::format("%.3f@%.1fs") % it->matched_mass() % it->tR() ).str()
                , it->matched_mass()
                , it->matched_mass_height()
                , int( idx )
                , int( it->matched_mass_height() ) };
    }
    ms->setMassArray( std::move( masses ) );
    ms->setIntensityArray( std::move( intens ) );
    ms->set_annotations( annots );

    return ms;
}

std::shared_ptr< adcontrols::MSPeakInfo >
summarizer::get() const
{
    return impl_->pInfo_;
}
