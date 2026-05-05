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

#include "lrpfile.hpp"
#include "instsetup.hpp"
#include "lrpcalib.hpp"
#include "lrphead2.hpp"
#include "lrphead3.hpp"
#include "lrpheader.hpp"
#include "lrptic.hpp"
#include "msdata.hpp"
#include "simions.hpp"
#include <adportable/debug.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <istream>
#include <sstream>

namespace shrader {

    class lrpfile::impl {
    public:
        impl() : loaded_( false )
               , header_( std::make_unique< lrpheader >() )
               , header2_( std::make_unique< lrphead2 >() )
               , header3_( std::make_unique< lrphead3 >() )
               , instsetup_( std::make_unique< shrader::instsetup >() )
               , lrpcalib_( std::make_unique< shrader::lrpcalib >() )
               , simions_( std::make_unique< shrader::simions >() )
               , lrptic_( std::make_unique< shrader::lrptic >() )
            {}
        bool loaded_;
        std::unique_ptr< shrader::lrpheader > header_;
        std::unique_ptr< shrader::lrphead2 > header2_;
        std::unique_ptr< shrader::lrphead3 > header3_;
        std::unique_ptr< shrader::instsetup > instsetup_;
        std::unique_ptr< shrader::lrpcalib > lrpcalib_;
        std::unique_ptr< shrader::simions > simions_;
        std::unique_ptr< shrader::lrptic > lrptic_;
        std::vector< std::shared_ptr< shrader::msdata > > msdata_;
    };
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

using namespace shrader;

lrpfile::~lrpfile()
{
}

lrpfile::lrpfile() : impl_( std::make_unique< impl >() )
{
}


bool
lrpfile::load( std::istream& in, size_t fsize )
{
    if ( impl_->header_->load( in, fsize ) &&
         impl_->header2_->load( in, fsize ) &&
         impl_->header3_->load( in, fsize ) ) {

        if ( not impl_->instsetup_->load( in, fsize ) )
            ADDEBUG() << "## instsetup load failed ##";
        if ( not impl_->lrpcalib_->load( in, fsize ) )
            ADDEBUG() << "## lrpcalib load failed ##";
        if ( not impl_->simions_->load( in, fsize ) )
            ADDEBUG() << "## simions load failed ##";
        if ( not impl_->lrptic_->load( in, fsize ) )
            ADDEBUG() << "## lrptic load failed ##";

        if ( impl_->lrptic_ && *impl_->lrptic_ ) {
            for ( auto& tic: impl_->lrptic_->tic() ) {
                if ( auto data = std::make_shared< shrader::msdata >() ) {
                    if ( data->load( in, fsize, tic.ptr ) ) {
                        impl_->msdata_.emplace_back( std::move( data ) );
                    }
                }
            }
        }
        return true;
    }
    return false;
}

bool
lrpfile::xload( value_type t, const std::string& data )
{
    std::istringstream in( data );
    std::visit(overloaded{
            [](auto arg) { std::cout << arg << ' '; }
                , [&](lrpheader) { impl_->header_->load( in, data.size() ); }
                , [&](lrphead2)  { impl_->header2_->load( in, data.size() ); }
                , [&](lrphead3)  { impl_->header3_->load( in, data.size() ); }
                , [&](class instsetup) { impl_->instsetup_->load( in, data.size() ); }
                , [&](class lrpcalib)  { impl_->lrpcalib_->load( in, data.size() ); }
                , [&](class simions)   { impl_->simions_->load( in, data.size() ); }
                }, t);
    return true;
}

void
lrpfile::append( std::string&& data, std::string&& meta )
{
    if ( auto p = std::make_shared< shrader::msdata >() ) {
        std::istringstream in( data );
        p->load( in, data.size(), 0 );
        impl_->msdata_.emplace_back( std::move( p ) );
    }
    impl_->lrptic_->append( std::move( meta ) );

}


lrpfile::operator bool() const
{
    return impl_->loaded_;
}

const shrader::msdata *
lrpfile::operator []( size_t idx ) const
{
    if ( impl_->msdata_.size() > idx )
        return impl_->msdata_[ idx ].get();
    return 0;
}

const shrader::lrpheader&
lrpfile::header() const
{
    return *impl_->header_;
}

const shrader::lrphead2&
lrpfile::header2() const
{
    return *impl_->header2_;
}

const shrader::lrphead3&
lrpfile::header3() const
{
    return *impl_->header3_;
}

const shrader::instsetup&
lrpfile::instsetup() const
{
    return *impl_->instsetup_;
}

const shrader::lrpcalib&
lrpfile::lrpcalib() const
{
    return *impl_->lrpcalib_;
}

const shrader::simions&
lrpfile::simions() const
{
    return *impl_->simions_;
}

const shrader::lrptic&
lrpfile::lrptic() const
{
    return *impl_->lrptic_;
}

const std::vector< std::shared_ptr< shrader::msdata > >&
lrpfile::msdata() const
{
    return impl_->msdata_;
}

size_t lrpfile::number_of_spectra() const
{
    return impl_->msdata_.size();
}

std::string
lrpfile::time_of_injection() const
{
    std::tm tm{};
    std::istringstream in( header().analdate() );
    in >> std::get_time( &tm, "%m/%d/%Y" );

    std::tm lt{0};
    time_t t = time(0);
    localtime_r(&t, &lt);

    // "analdate":"5/30/2025" "analtime":"17:17:24" --> 2025-05-30T17:17:24+0900 (in case at JST area)
    return std::format( "{:04d}-{:02d}-{:02d}T{}{:+03d}{:02d}"
                        , tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday
                        , header().analtime()
                        , lt.tm_gmtoff / 3600
                        , (lt.tm_gmtoff / 60) % 60 );
}

ticc_t
lrpfile::get_ticc() const
{
    if ( lrptic() && lrptic() ) {
        ticc_t ticc;
        for ( auto& tic : lrptic().tic() )
            ticc.emplace_back( double( tic.time ) / 1000.0, tic.intensity, tic.ptr, tic.overload );
        return ticc;
    }
    return {};
}

void
lrpfile::tic_set_loaded( bool f )
{
    impl_->lrptic_->set_loaded( f );
}

void
lrpfile::dump( std::ostream& of, size_t limit ) const
{
    if ( impl_->header_ && header() ) {
        of << "---------------------- header ------------------------" << std::endl;
        of << "flags: " << header().flags() << std::endl;
        of << boost::format( "version: %1%\t%2%" ) % header().version() % header().data_type_code() << std::endl;
        of << boost::format( "analdate: %1%" ) % header().analdate() << std::endl;
        of << boost::format( "analtime: %1%" ) % header().analtime() << std::endl;
        of << boost::format( "instrument: %1%" ) % header().instrument() << std::endl;
        of << boost::format( "operator: %1%" ) % header().operator_name() << std::endl;
        of << boost::format( "calfile: %1%" ) % header().calfile() << std::endl;
        of << boost::format( "library: %1%" ) % header().library() << std::endl;
        of << boost::format( "libcaldate: %1%" ) % header().libcaldate() << std::endl;
        of << boost::format( "interfacetype: %1%\t%2%" ) % header().interfacetype() % header().interfacetype_code() << std::endl;
        of << boost::format( "rawdatatype: %1%\t%2%" ) % header().rawdatatype() % header().rawdatatype_code() << std::endl;
        of << boost::format( "secondDemention: %1%" ) % header().SecondDmension() << std::endl;
        of << boost::format( "AltTicPtr: %1%" ) % header().AltTicPtr() << std::endl;
        of << boost::format( "nscans: %1%" ) % header().nscans() << std::endl;
        of << boost::format( "setupptr: %1%" ) % header().setupptr() << std::endl;
        of << boost::format( "calptr: %1%" ) % header().calptr() << std::endl;
        of << boost::format( "simptr: %1%" ) % header().simptr() << std::endl;
        of << boost::format( "scanptr: %1%" ) % header().scanptr() << std::endl;
        of << boost::format( "ticptr: %1%" ) % header().ticptr() << std::endl;
        of << boost::format( "miscptr: %1%" ) % header().miscptr() << std::endl;
        of << boost::format( "labelptr %1%" ) % header().labelptr() << std::endl;
    }
    if ( impl_->header2_ && header2() ) {
        of << "---------------------- header2 ------------------------" << std::endl;
        of << "flags: " << header2().flags() << std::endl;
        of << boost::format( "descline1: %1%" ) % header2().descline1() << std::endl;
        of << boost::format( "descline2: %1%" ) % header2().descline2() << std::endl;
        of << boost::format( "client: %1%" ) % header2().client() << std::endl;
        auto pistd = impl_->header2_->istd();
        of << "istd (ng): ";
        for ( size_t i = 0; i < 13; ++i )
            of << pistd[ i ] << ", ";
        of << std::endl;
    }

    if ( impl_->header3_ && header3() ) {
        of << "---------------------- header3 ------------------------" << std::endl;
        of << "flags: " << header3().flags() << std::endl;
        of << boost::format( "installation title: %1%" ) % header3().instaltitle() << std::endl;
        of << boost::format( "inlet: %1%" ) % header3().inlet() << std::endl;
        of << boost::format( "comments: %1%" ) % header3().comments() << std::endl;
    }

    if ( impl_->instsetup_ && instsetup() ) {
        of << "---------------------- instsetup ------------------------" << std::endl;
        of << "flags:\t" << instsetup().flags() << std::endl;
        of << "ionization:\t" << instsetup().ionization() << std::endl;
        of << "\tIonization method code:\t" << instsetup().describe_ionization() << std::endl;
        of << "#### upperdrive:\t" << instsetup().upperdrive() << "\tUpper mass drive" << std::endl;
        of << "#### lowerdrive:\t" << instsetup().lowerdrive() << "\tLower mass drive" << std::endl;
        of << "lmasslim:\t" << double(instsetup().lmasslim()) / 65536 << "\tLower mass limit of scan" << std::endl;
        of << "umasslim:\t" << double(instsetup().umasslim()) / 65536 << "\tUpper mass limit of scan" << std::endl;
        of << "ucallim:\t" << double(instsetup().ucallim()) / 65536 << "\tUpper mass limit of calibration" << std::endl;
        of << "lcallim:\t" << double(instsetup().lcallim()) / 65536 << "\tLower mass limit of calibration" << std::endl;
        of << "aves:\t" << instsetup().aves() << "\tNumber A/D readings per D/A step" << std::endl;
        of << "#### stepsize:\t" << instsetup().stepsize() << "\tStep size between data points" << std::endl;
        of << "scanspeed:\t" << instsetup().scanspeed() << "\tScan/second" << std::endl;
        of << "scancycle:\t" << instsetup().scancycle() << "\tInterscan delay (msec)" << std::endl;
        of << "caltable:\t" << instsetup().caltable() << "\tCalibration table used" << std::endl;
        of << "scanmode:\t" << instsetup().scanmode() << "\tScanning filed code:\t" << instsetup().describe_scanmode() << std::endl;
        of << "scanlaw:\t" << instsetup().scanlaw() << "\tScan law code:\t" << instsetup().describe_scanlaw() << std::endl;
        of << "resolution:\t" << instsetup().resolution() << "\tInstrument resolution" << std::endl;
        of << "reswindow:\t" << instsetup().reswindow() << "\tPeak width used for peak detection" << std::endl;
        of << "calslope:\t" << instsetup().calslope() << "\tCalibration slope (linear scan only)" << std::endl;
        of << "calinter:\t" << instsetup().calinter() << "\tCalibration intercept(linear scan only)" << std::endl;
        of << "#### clockbaud:\t" << instsetup().clockbaud() << "\tClock baud rate in seconds/data point" << std::endl;
        of << "overload:\t" << instsetup().overload() << "\tMaximum intensity(A/D max - baseline value)" << std::endl;
        of << "timewindow:\t" << instsetup().timewindow() << "\tnot used" << std::endl;
        of << "masswindow:\t" << instsetup().masswindow() << "\tMass window for selected ion monitoring" << std::endl;
        of << "inttime:\t" << instsetup().inttime() << "\tIntegration time for selected ion monitoring" << std::endl;
        of << "method:\t" << instsetup().method() << "\tMethod name" << std::endl;
        of << "autosamproc:\t" << instsetup().autosamproc() << "\tAutosampler procedure name" << std::endl;
        of << "gcproc:\t" << instsetup().gcproc() << "\tGC procedure name" << std::endl;
        of << "TOFDrift:\t" << instsetup().TOFDrift() << "\tTOF correction factor" << std::endl;
        of << "samplesize:\t" << instsetup().samplesize() << "\tSample size" << std::endl;
        of << "sampleunits:\t" << instsetup().sampleunits() << "\tSample size units" << std::endl;
        of << "peakcentroid:\t" << instsetup().peakcentroid() << "\tCentroiding method:\t" << instsetup().describe_peakcentroid() << std::endl;
        of << "pkintensity:\t" << instsetup().pkintensity() << "\tIntensity method (0 = height, 1 = area)" << std::endl;
        of << "inithreshold:\t" << instsetup().inithreshold() << "\tThreshold at low mass (as A/D value)" << std::endl;
        of << "fnlthreshold:\t" << instsetup().fnlthreshold() << "\tThreshold at high mass (as A/D value)" << std::endl;
        of << "HVolt:\t" << instsetup().HVolt() << "\tAccelerating voltage" << std::endl;
        of << "HVscanBValue:\t" << instsetup().HVscanBValue() << "\tCalibration intercept for HV scan" << std::endl;
        of << "Peakthres:\t" << instsetup().Peakthres() << "\tCentroiding algorithm threshod(%)" << std::endl;
        of << "baseline:\t" << instsetup().baseline() << "\tMeasured instrument baseline" << std::endl;
        of << "noise:\t" << instsetup().noise() << "\tMeassured instrument baseline noise" << std::endl;
        of << "linkcorrection:\t" << instsetup().linkcorrection() << "\tMass correction for linked scans" << std::endl;
        of << "valley:\t" << instsetup().valley() << "\tCentroiding algorithm valle(%)" << std::endl;
        of << "minpeakwidth:\t" << instsetup().minpeakwidth() << "\tCentroiding algorithm minimum peak width(%)" << std::endl;
        of << "sampletype:\t" << instsetup().sampletype() << "\t0:solid, 1:Solid by Dry Weight, 2: Liquid, 3: Gas" << std::endl;
        of << "unitscode:\t" << instsetup().unitscode() << "\tSample size units( 0:ug, 3:Kg)" << std::endl;
        of << "dryweight:\t" << instsetup().dryweight() << "\tPercent dry weight" << std::endl;
        of << "linkmass:\t" << instsetup().linkmass() << "\tLink mass" << std::endl;
        of << "SIMfield:\t" << instsetup().SIMfield() << "\tSwitching Field for SIM" << std::endl;
        of << "SIMBset:\t" << instsetup().SIMBset() << "\tMagnet Field reference value used for EF SIM" << std::endl;
        of << "SIMBfield:\t" << instsetup().SIMBfield() << "\tMagnet field value used for EF SIM" << std::endl;
        of << "Slitcouple:\t" << instsetup().Slitcouple() << "\tSlits coupled?" << std::endl;
        of << "Maxmassrange:\t" << instsetup().Maxmassrange() << "\tMaximum mass range for instrument" << std::endl;
        of << "HvscanBDrive:\t" << instsetup().HvscanBDrive() << "\tMagnet field reference used of HV scan" << std::endl;
        of << "SIMCalOK:\t" << instsetup().SIMCalOK() << "\tSIM calibration OK" << std::endl;
        of << "Maxvolt:\t" << instsetup().Maxvolt() << "\tMaximum High voltage for instrument" << std::endl;
        of << "PeakFilter:\t" << instsetup().PeakFilter() << "\t" << std::endl;
        of << "TwoWayScan:\t" << instsetup().TwoWayScan() << "\t" << std::endl;
    }
    of << std::endl;

    if ( impl_->lrpcalib_ && lrpcalib() ) {
        of << "---------------------- lrpcalib ------------------------" << std::endl;
        of << "flags:\t" << impl_->lrpcalib_->flags() << std::endl;
        for ( int i = 0; i < impl_->lrpcalib_->cal_size; ++i ) {
            auto cal = impl_->lrpcalib_->cal_data( i );
            of << std::format( "\tmass: {:.7g}\tintensity: {:7g}\tcoeff_a: {:7g}\tcoeff_b: {:7g}\n"
                               , std::get<cal_mass>(cal) / 65536.0
                               , std::get<cal_intens>(cal)
                               , std::get<cal_coeff_a>(cal)
                               , std::get<cal_coeff_b>(cal) );
        }
        of << "type:\t" << impl_->lrpcalib_->type() << std::endl;
    }
#if 0
    if ( impl_->lrptic_ && lrptic() ) {
        of << "---------------------- lrptic ------------------------" << std::endl;
        of << "----------- TIC -------------- " << impl_->lrptic_->tic().size() << std::endl;
        of << "nexttpr:\t" << impl_->lrptic_->nextptr() << std::endl;
        for ( auto& tic: impl_->lrptic_->tic() ) {
            of << boost::format( "\ttime: %.3fs\tintens: %d\toffs: 0x%x\toverload: %d\n" )
                % (double( tic.time ) / 1000.0) % tic.intensity % tic.ptr % tic.overload;
        }
        of << "----------- END of TIC --------------" << std::endl;
    }
    of << std::endl;
#endif
    of << "---------------------- msdata ------------------------" << std::endl;
    of << "total: " << impl_->msdata_.size() << " spectra." << std::endl;
    size_t scan = 0;
    for ( auto& msdata : impl_->msdata_ ) {
        if ( scan >= limit )
            break;
        if ( msdata ) {
            size_t nblocks = msdata->size();
            of << "scan: " << ++scan;
            if ( msdata->is_profile( msdata->flags( 0 ) ) ) {
                of << " profile ";
            }
            else {
                of << " centroid (or SIM, SRM)";
            }
            of << std::endl;

            for ( int blk = 0; blk < nblocks; ++blk ) {
                of << boost::format( "blk# %2d\tscan# %3d\tnions: %2d" ) % blk % msdata->scan( blk ) % msdata->nions( blk );
                auto xrange = std::make_pair( msdata->xlow( blk ), msdata->xhigh( blk ) );
                auto flags = msdata->flags( blk );
                auto nions = msdata->nions( blk );

                of << boost::format( "\tx-range: (%5d, %5d) d = %2d" )
                    % xrange.first % xrange.second % (xrange.second - xrange.first);

                if ( msdata->is_mass_array( flags ) ) {
                    of << " := m/z(" << double( xrange.first ) / 65536 << ", " << double( xrange.second ) / 65536 << ")";
                } else {
                    auto range = std::make_pair( double( xrange.first ) / 16, double( xrange.second ) / 16 );
                    of << boost::format( "\t:= time( %.3f, %.3f ) dt: %.7e" )
                        % range.first % range.second % ( ( range.second - range.first ) / ( nions - 1 ) );
                }
                of << std::endl;
            }
        }
    }
}
