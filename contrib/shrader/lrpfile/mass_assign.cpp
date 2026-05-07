/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "mass_assign.hpp"
#include "lrpfile.hpp"
#include "instsetup.hpp"
#include "lrpcalib.hpp"
#include <adportable/debug.hpp>
#include <format>

namespace shrader {

    class mass_assign::impl {
    public:
        impl() : dt_( 0.5e-9 ), cal_{} {
        }
        impl( const lrpfile& file ) : dt_( file.instsetup().stepsize() * file.instsetup().clockbaud() * 1.0e-9 )
                                    , cal_( file.lrpcalib().cal_data( 0 ) ) {
        }

        double dt_;
        cal_data cal_;
    };

}

using namespace shrader;

mass_assign::~mass_assign()
{
}

mass_assign::mass_assign() : impl_( std::make_unique< impl >() )
{
}

mass_assign::mass_assign( const class lrpfile& file )
    : impl_( std::make_unique< impl >( file ) )
{
}

double
mass_assign::time( size_t idx ) const
{
    return double(idx) * impl_->dt_ * 1.0e-9;
}

double
mass_assign::operator()( std::size_t idx ) const
{
    const auto a = std::get< cal_coeff_a >( impl_->cal_ );  // seconds
    const auto b = std::get< cal_coeff_b >( impl_->cal_ );  // seconds / sqrt(Da)
    const auto adc_delay = std::get< cal_mass >( impl_->cal_ );  // seconds / sqrt(Da)
    const double t = double (adc_delay + idx ) * impl_->dt_;

    if ( t <= 0 || b == 0 ) {
        return 0.0;
    }

    // const double x = (t - a) / b;
    const double x = (t - a) / b;
    if ( idx < 20 )
        ADDEBUG() << "mass_assign: " << std::format( "idx={}, t={} --> mass={}", idx, t, x*x )
                  << "\t" << impl_->cal_;

    return x * x;
}

#if 0
double
mass_assign::mass( size_t idx ) const
{

    const double t_ns = double(idx) * impl_->dt_ns_;
    const double dt = t_ns - std::get< cal_coeff_a >( impl_->cal_ );
    auto m = std::get< cal_coeff_b >( impl_->cal_ ) * dt * dt;

    ADDEBUG() << "mass_assign: " << std::format( "t_ns={}, idx={}, dt={}, a={}, b={}, m={}"
                                                 , t_ns
                                                 , idx
                                                 , dt
                                                 , std::get< cal_coeff_a >( impl_->cal_ )
                                                 , std::get< cal_coeff_b >( impl_->cal_ )
                                                 , m );
    return m;
}
#endif
