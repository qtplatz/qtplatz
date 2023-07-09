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

#include "noise_filter.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/digital_filter.hpp>
#include <adportable/fft4g.hpp>

namespace adprocessor {

    class noise_filter::impl {
    public:
        adcontrols::ProcessMethod pm_;
        adportable::fft4g fft_;
    };

}


using namespace adprocessor;

noise_filter::~noise_filter()
{
}

noise_filter::noise_filter() : impl_( std::make_unique< impl >() )
{

}


std::shared_ptr< adcontrols::Chromatogram >
noise_filter::operator()( const adcontrols::Chromatogram& c, double freq ) const
{
    using namespace adportable;

    auto pchr = std::make_shared< adcontrols::Chromatogram >( c );
    size_t N = adportable::digital_filter::make_power2::size2( pchr->size() );
    std::vector< std::complex< double > > d( N );

    digital_filter::make_power2::transform( pchr->getIntensityArray()
                                                        , pchr->getIntensityArray() + pchr->size()
                                                        , d.begin()
                                                        , [](const auto& a){ return std::complex<double>(a); } );
    impl_->fft_.cdft( 1, d );

    size_t index = digital_filter::cutoff_index()( pchr->sampInterval(), N, freq );
    digital_filter::filter().low_pass( d, index );

    impl_->fft_.cdft( -1, d );

    for ( size_t i = 0; i < pchr->size(); ++i )
        pchr->setIntensity( i, d[i].real() / N );

    return pchr;
}
