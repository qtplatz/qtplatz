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
        impl() : actualPoints_( 1000 * 10 ) // 10us
               , xIncrement_( 1.0e-9 )
               , delay_( 0.0 )
               , dist_( -3, 5 )
               , sign_( -1 )
               , sigma_( 1.0e-9 )
               , pseudo_peaks_( { { 1.0e-6, 200 }, { 1.5e-6, 400 }, { 2.0e-6, 2000 }, { 2.5e-6, 100 } } ) {

            const double sigma_( 1.0e-9 );
            
        }

        std::shared_ptr< mblock< int16_t > > mblk_;
        double sigma_;
        double delay_;
        double xIncrement_;
        size_t actualPoints_;
        int sign_;
        std::mt19937 __gen__;
        std::uniform_real_distribution<double> dist_;
        std::vector< std::pair< double, int > > pseudo_peaks_;

        static impl& instance() {
            static impl __impl__;
            return __impl__;
        }

        double noise() {
            return dist_( __gen__ );
        };

        void generate() {

            if ( mblk_ )
                return;

            mblk_ = std::make_shared< mblock< int16_t > >( actualPoints_ );
            for ( size_t i = 0; i < actualPoints_; ++i ) {
                double t = /* delay_ + */ xIncrement_ * i;
                double y(0);
                for ( auto& peak: pseudo_peaks_ ) {
                    if ( std::abs( t - peak.first ) < 3.0e-9 ) {
                        boost::math::normal_distribution< double > nd( peak.first, sigma_ );
                        y += boost::math::pdf( nd, t ) * sigma_ * peak.second * sign_;
                    }
                }
                mblk_->data()[ i ] = int( y );
            }
        }

        template< typename value_type >
        void generate( std::shared_ptr< mblock<value_type> >& mblk, int numRecords ) {

            mblk = std::make_shared < adportable::mblock< value_type > >( actualPoints_ * numRecords );
            generate();
            
            auto * dp = mblk->data();
            
            for ( int i = 0; i < numRecords; ++i ) {
                auto * sp = mblk_->data();
                std::transform( sp, sp + actualPoints_, dp, [] ( int y ) { return y + impl::instance().noise(); } );
                dp += impl::instance().actualPoints_;
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

waveform_simulator::waveform_simulator( double delay, size_t size, double xIncrement )
{
    if ( impl::instance().actualPoints_ != size ||
         std::abs( impl::instance().delay_ - delay ) >= 1.0e-10 ||
         std::abs( impl::instance().xIncrement_ - xIncrement ) >= 1.0e-10 )
        impl::instance().mblk_.reset();
    
    impl::instance().actualPoints_ = size;
    impl::instance().delay_ = delay;
    impl::instance().xIncrement_ = xIncrement;
}

void
waveform_simulator::operator () ( std::shared_ptr< mblock<int16_t> >& mblk, int numRecords ) const
{
    impl::instance().generate< int16_t >( mblk, numRecords );    
}

void
waveform_simulator::operator () ( std::shared_ptr< mblock<int32_t> >& mblk, int numRecords ) const
{
    impl::instance().generate< int32_t >( mblk, numRecords );
}

size_t
waveform_simulator::actualPoints() const
{
    return impl::instance().actualPoints_;
}
