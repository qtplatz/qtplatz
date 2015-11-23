// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "waveform_simulator.hpp"
#include <boost/math/distributions/normal.hpp>
#include <random>

namespace adportable {

    class waveform_simulator::impl {
    public:
        impl() : size_( 0 )
               , xIncrement_( 1.0e-9 )
               , width_( 10.0e-6 )
               , delay_( 0.0 )
               , dist_( -2, 3 )
               , sign_( -1 )
               , pseudo_peaks_( { { 1.0e-6, 0x1f }, { 1.5e-6, 0x1f }, { 2.0e-6, 0x1f } } ) {
            
        }

        std::vector< std::pair< double, int > > pseudo_peaks_;
        std::shared_ptr< mblock< int16_t > > mblk_;
        size_t size_;
        double xIncrement_;
        double width_;
        double delay_;
        int sign_;
        std::mt19937 __gen__;
        std::uniform_real_distribution<double> dist_;

        static impl& instance() {
            static impl __impl__;
            return __impl__;
        }

        double noise() {
            return dist_( __gen__ );
        };

        void generate() {
            size_ = size_t( width_ / xIncrement_ + 0.5 );
            mblk_ = std::make_shared< mblock< int16_t > >( size_ );
            for( size_t i = 0; i < size_; ++i ) {
                double t = delay_ + xIncrement_ * i;
                double y;
                for ( auto& peak: pseudo_peaks_ ) {
                    boost::math::normal_distribution< double > nd( peak.first, 1.0e-9 );
                    y += boost::math::pdf( nd, t ) * peak.second * sign_;
                }
                mblk_->data()[ i ] = int( y );
            }
        }
    };

}

using namespace adportable;

waveform_simulator::~waveform_simulator()
{
}

waveform_simulator::waveform_simulator()
{
}

void
waveform_simulator::operator () ( std::shared_ptr< mblock<int16_t> >& mblk, int numRecords ) const
{
    if ( ! impl::instance().mblk_ )
        impl::instance().generate();

    mblk = std::make_shared< mblock<int16_t > >( impl::instance().size_ * numRecords );

    auto * dp = mblk->data();
    for ( int i = 0; i < numRecords; ++i ) {
        auto * sp = impl::instance().mblk_->data();
        std::transform( sp, sp + impl::instance().size_, dp, []( int y ){ return y + impl::instance().noise(); } );
        dp += impl::instance().size_;
    }
}

void
waveform_simulator::operator () ( std::shared_ptr< mblock<int32_t> >& mblk, int numRecords ) const
{
    if ( ! impl::instance().mblk_ )
        impl::instance().generate();

    auto * dp = mblk->data();
    for ( int i = 0; i < numRecords; ++i ) {
        auto * sp = impl::instance().mblk_->data();
        std::transform( sp, sp + impl::instance().size_, dp, []( int y ){ return y + impl::instance().noise(); } );
        dp += impl::instance().size_;
    }
}

size_t
waveform_simulator::actualPoints() const
{
    return impl::instance().size_;
}
