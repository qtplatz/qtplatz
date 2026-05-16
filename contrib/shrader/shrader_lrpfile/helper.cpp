// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC
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

#include "helper.hpp"
#include <lrpfile.hpp>
#include <instsetup.hpp>
#include <lrpcalib.hpp>
#include <lrpfile.hpp>
#include <lrphead2.hpp>
#include <lrphead3.hpp>
#include <lrpheader.hpp>
#include <lrptic.hpp>
#include <msdata.hpp>
#include <simions.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/base64.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/debug.hpp>
#include <adportable/iso8601.hpp>
#include <memory>
#include <utility>

namespace shrader {

    std::string bzip2_compress( std::string&& data ) {
        std::string compressed;
        adportable::bzip2::compress( compressed, data.data(), data.size() );
        return compressed;
    }

    std::string bzip2_decompress( const std::string& compressed ) {
        if ( adportable::bzip2::is_a( compressed.data(), compressed.size() ) ) {
            std::string inflated;
            adportable::bzip2::decompress( inflated, compressed.data(), compressed.size() );
            return inflated;
        }
        return compressed;
    }

    std::string bzip2_decompress( adfs::blob blob ) {
        return bzip2_decompress( std::string{ reinterpret_cast< const char * >(blob.data()), blob.size() } );
    }

    std::string block_to_string( const std::array< char, 256 >& data ) {
        return base64_encode( bzip2_compress( std::string( data.data(), data.size() ) ) );
    }

    std::string string_to_block( const std::string& data ) {
        return bzip2_decompress( base64_decode( data ) );
    }

} // namespace mzml

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

namespace shrader {

    std::shared_ptr< adcontrols::MassSpectrum >
    make_spectrum( int64_t rowid, const lrpfile& lrpfile ) {

        auto header_mass_range = [&]()->std::pair<double,double>{
            return {double(lrpfile.instsetup().lmasslim())/65536.0
                    , double( lrpfile.instsetup().umasslim()) /65536.0 };
        };

        const auto upperdrive = lrpfile.instsetup().upperdrive();
        const auto stepsize  = lrpfile.instsetup().stepsize();
        const auto clockbaud = lrpfile.instsetup().clockbaud();
        const auto scanlaw = lrpfile.instsetup().scanlaw();
        double fSampInterval = (stepsize * clockbaud) * 1.0e-9;

        // ADDEBUG() << "############# spectral time step: "
        //           << std::format( "{}, stepsize={}, clockbaud={}", fSampInterval, stepsize, clockbaud );

        if ( rowid > lrpfile.msdata().size() )
            return {};

        if ( rowid < 0 || rowid >= lrpfile.msdata().size()) {
            return {};
        }
        auto y = lrpfile.msdata().at( rowid )->intensities();
        auto numSamples = y.size();
        auto mass_range = header_mass_range();

        // check shrader::massspectrometer class -- also has same code
        auto mass_at = [&]( size_t idx )->double{
            // assume time squared scan law (TOF Eq.)
            const double f = double( idx ) / double( numSamples - 1 );
            const double s0 = std::sqrt( mass_range.first );
            const double s1 = std::sqrt( mass_range.second );
            const double s = s0 + f * ( s1 - s0 );
            return s * s;
        };

        // mass_assign class is not yet working -- asking to KANOMAX for CAL interpretation
        // shrader::mass_assign mass_assigner( *impl_->lrpfile_ );

        std::vector< std::pair< double, double > > vec;
        for ( size_t i = 0; i < y.size(); ++i ) {
            vec.emplace_back( mass_at( i ), y.at( i ) );
        }

        if ( auto ms = std::make_shared< adcontrols::MassSpectrum >() ) {
            for ( const auto& datum: vec )
                *ms << datum;
            ms->setAcquisitionMassRange( mass_range.first, mass_range.second );
            ms->getMSProperty().setTrigNumber( lrpfile.msdata().at( rowid )->blocks().at(0).scan );
            ms->getMSProperty().setInstMassRange( header_mass_range() );
            if ( auto& lrptic = lrpfile.lrptic() ) {
                const auto milliseconds = lrptic.tic().at( rowid ).time;
                ms->getMSProperty().setTimeSinceInjection( milliseconds, adcontrols::metric::milli );
                auto time_of_injection = lrpfile.time_of_injection();
                if ( auto tp = adportable::iso8601::parse( time_of_injection.begin(), time_of_injection.end() ) ) {
                    ms->getMSProperty().setTimePoint( *tp + std::chrono::milliseconds( milliseconds ) );
                }
            }

            double delayT = int((std::sqrt( mass_range.first ) * 1.0e-6 / fSampInterval)) * fSampInterval; // workaround
            adcontrols::SamplingInfo sInfo(
                /* double fSampInterval */ fSampInterval // seconds
                , /* double delayTime   */ delayT // workaround
                , /* int32_t nDelay     */ int32_t( delayT / fSampInterval )
                , /* uint32_t nSamples  */ numSamples
                , /* uint32_t nAvg      */ 1      // number of triggers to be averaged := not known
                , /* uint32_t mode      */ 10 );  // nlaps for infitof, set arbitrary number

            // ms->getMSProperty().setSamplingInfo( sInfo );

            return ms;
        }
        return {};
    }

}
