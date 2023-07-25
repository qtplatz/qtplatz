
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
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adportable/debug.hpp>

namespace adprocessor {
    namespace jcb2009_helper {

        class summarizer::impl {
        public:
            impl() {
            }
            std::shared_ptr< adcontrols::MassSpectrum > pSummary_;
            std::shared_ptr< adcontrols::MSPeakInfo > pInfoSummary_;
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
summarizer::operator()( std::shared_ptr< const adcontrols::MassSpectrum > pCentroid
                        , std::shared_ptr< const adcontrols::MSPeakInfo > pInfo )
{
    (*this)(pCentroid);
    (*this)(pInfo);
}


void
summarizer::operator()( std::shared_ptr< const adcontrols::MassSpectrum > pCentroid )
{
    typedef adcontrols::MassSpectrum T;

    if ( !impl_->pSummary_ ) {
        impl_->pSummary_ = std::make_shared< adcontrols::MassSpectrum >( *pCentroid );
        impl_->pSummary_->addDescription( adcontrols::description( L"create", L"Summary" ) );

        for ( auto& ms: adcontrols::segment_wrapper< T >( *impl_->pSummary_ ) )
            ms.resize( 0 );
    }

    auto dIt = adcontrols::segment_wrapper< T >( *impl_->pSummary_ ).begin();

    for ( auto& ms: adcontrols::segment_wrapper< const T >( *pCentroid ) ) {
        for ( auto anno: ms.get_annotations() ) {
            if ( anno.index() >= 0 ) {
                size_t idx = (*dIt) << std::make_pair( ms.mass( anno.index() ), ms.intensity( anno.index() ) );
                anno.index( int(idx) ); // replace index
                (*dIt).get_annotations() << anno;
            }
        }
        ++dIt;
    }
}

void
summarizer::operator()( std::shared_ptr< const adcontrols::MSPeakInfo > pInfo )
{
    typedef adcontrols::MSPeakInfo T;

    if ( !impl_->pInfoSummary_ ) {
        impl_->pInfoSummary_ = std::make_shared< adcontrols::MSPeakInfo >( *pInfo );
        for ( auto& info: adcontrols::segment_wrapper< T >( *impl_->pInfoSummary_ ) )
            info.clear();
    }

    auto dIt = adcontrols::segment_wrapper< T >( *impl_->pInfoSummary_ ).begin();
    for ( auto& info: adcontrols::segment_wrapper< const T >( *pInfo ) ) {
        for ( auto item: info ) {
            if ( !item.annotation().empty() ) {
                auto idx = (*dIt).size();
                item.set_peak_index( idx );
                (*dIt) << item;
            }
        }
        ++dIt;
    }
}

std::pair< std::shared_ptr< adcontrols::MassSpectrum >
           , std::shared_ptr< adcontrols::MSPeakInfo > >
summarizer::get() const
{
    return { impl_->pSummary_, impl_->pInfoSummary_ };
}
