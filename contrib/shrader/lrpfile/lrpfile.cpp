/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "lrpheader.hpp"
#include "lrphead2.hpp"
#include "lrphead3.hpp"
#include "instsetup.hpp"
#include "lrpcalib.hpp"
#include "simions.hpp"
#include "lrptic.hpp"
#include "msdata.hpp"
#include <boost/format.hpp>

using namespace shrader;

lrpfile::~lrpfile()
{
}

lrpfile::lrpfile( std::istream& in, size_t fsize ) : loaded_( false )
{
    header_ = std::make_shared< shrader::lrpheader >( in, fsize );
    header2_ = std::make_shared< shrader::lrphead2 >( in, fsize );
    header3_ = std::make_shared< shrader::lrphead3 >( in, fsize );
    instsetup_ = std::make_shared< shrader::instsetup >( in, fsize );
    lrpcalib_ = std::make_shared< shrader::lrpcalib >( in, fsize );
    simions_ = std::make_shared< shrader::simions >( in, fsize );
    if ( ! (*simions_ ) )
        simions_.reset();
    lrptic_ = std::make_shared< shrader::lrptic >( in, fsize );
    if ( *lrptic_ ) {
        for ( auto& tic: lrptic_->tic() ) {
            in.seekg( tic.ptr );
            msdata_.push_back( std::make_shared< shrader::msdata >( in, fsize ) );
        }
    }
}

lrpfile::operator bool() const
{
    return loaded_;
}

void
lrpfile::dump( std::ostream& of ) const
{
    if ( header_ && *header_ ) {
        of << "flags: " << header_->flags() << std::endl;
        of << "version: " << header_->version() << std::endl;
        of << "type: " << header_->type() << "\t";
        header_->data_type_code( of );
        of << "analdate: " << header_->analdate() << std::endl;
        of << "analtime: " << header_->analtime() << std::endl;
        of << "instrument: " << header_->instrument() << std::endl;
        of << "operator: " << header_->operator_name() << std::endl;
        of << "calfile: " << header_->calfile() << std::endl;
        of << "library: " << header_->library() << std::endl;
        of << "libcaldate: " << header_->libcaldate() << std::endl;
        of << "interfacetype: " << header_->interfacetype() << "\t";
        header_->interfacetype_code( of );
        of << "rawdatatype: " << header_->rawdatatype() << "\t";
        header_->rawdatatype_code( of );
        of << "secondDemention: " << header_->SecondDmension() << std::endl;
        of << "AltTicPtr: " << header_->AltTicPtr() << std::endl;
        of << "nscans: " << header_->nscans() << std::endl;
        of << "setupptr: " << header_->setupptr() << std::endl;
        of << "calptr: " << header_->calptr() << std::endl;
        of << "simptr: " << header_->simptr() << std::endl;
        of << "scanptr: " << header_->scanptr() << std::endl;
        of << "ticptr: " << header_->ticptr() << std::endl;
        of << "miscptr: " << header_->miscptr() << std::endl;
        of << "labelptr" << header_->labelptr() << std::endl;
    }
    if ( header2_ && *header2_ ) {
        of << "flags: " << header2_->flags() << std::endl;
        of << "descline1: " << header2_->descline1() << std::endl;
        of << "descline2: " << header2_->descline2() << std::endl;
        of << "client: " << header2_->client() << std::endl;
        auto pistd = header2_->istd();
        of << "istd (ng): ";
        for ( size_t i = 0; i < 13; ++i )
            of << pistd[ i ] << ", ";
        of << std::endl;
    }

    if ( header3_ && *header3_) {
        of << "flags: " << header3_->flags() << std::endl;
        of << "installation title: " << header3_->instaltitle() << std::endl;
        of << "inlet: " << header3_->inlet() << std::endl;
        of << "comments: " << header3_->comments() << std::endl;
    }

    if ( instsetup_ && *instsetup_ ) {
        of << "flags:\t" << instsetup_->flags() << std::endl;
        of << "ionization:\t" << instsetup_->ionization() << std::endl;
        of << "\tIonization method code:\t" << instsetup_->describe_ionization() << std::endl;
        of << "upperdrive:\t" << instsetup_->upperdrive() << "\tUpper mass drive" << std::endl;
        of << "lowerdrive:\t" << instsetup_->lowerdrive() << "\tLower mass drive" << std::endl;
        of << "umasslim:\t" << double(instsetup_->umasslim()) / 65536 << "\tUpper mass limit of scan" << std::endl;
        of << "lmasslim:\t" << double(instsetup_->lmasslim()) / 65536 << "\tLower mass limit of scan" << std::endl;
        of << "ucallim:\t" << double(instsetup_->ucallim()) / 65536 << "\tUpper mass limit of calibration" << std::endl;
        of << "lcallim:\t" << double(instsetup_->lcallim()) / 65536 << "\tLower mass limit of calibration" << std::endl;
        of << "aves:\t" << instsetup_->aves() << "\tNumber A/D readings per D/A step" << std::endl;
        of << "stepsize:\t" << instsetup_->stepsize() << "\tStep size between data points" << std::endl;
        of << "scanspeed:\t" << instsetup_->scanspeed() << "\tScan/second" << std::endl;
        of << "scancycle:\t" << instsetup_->scancycle() << "\tInterscan delay (msec)" << std::endl;
        of << "caltable:\t" << instsetup_->caltable() << "\tCalibration table used" << std::endl;
        of << "scanmode:\t" << instsetup_->scanmode() << "\tScanning filed code:\t" << instsetup_->describe_scanmode() << std::endl;
        of << "scanlaw:\t" << instsetup_->scanlaw() << "\tScan law code:\t" << instsetup_->describe_scanlaw() << std::endl;
        of << "resolution:\t" << instsetup_->resolution() << "\tInstrument resolution" << std::endl;
        of << "reswindow:\t" << instsetup_->reswindow() << "\tPeak width used for peak detection" << std::endl;
        of << "calslope:\t" << instsetup_->calslope() << "\tCalibration slope (linear scan only)" << std::endl;
        of << "calinter:\t" << instsetup_->calinter() << "\tCalibration intercept(linear scan only)" << std::endl;
        of << "clockbaud:\t" << instsetup_->clockbaud() << "\tClock baud rate in seconds/data point" << std::endl;
        of << "overload:\t" << instsetup_->overload() << "\tMaximum intensity(A/D max - baseline value)" << std::endl;
        of << "timewindow:\t" << instsetup_->timewindow() << "\tnot used" << std::endl;
        of << "masswindow:\t" << instsetup_->masswindow() << "\tMass window for selected ion monitoring" << std::endl;
        of << "inttime:\t" << instsetup_->inttime() << "\tIntegration time for selected ion monitoring" << std::endl;
        of << "method:\t" << instsetup_->method() << "\tMethod name" << std::endl;
        of << "autosamproc:\t" << instsetup_->autosamproc() << "\tAutosampler procedure name" << std::endl;
        of << "gcproc:\t" << instsetup_->gcproc() << "\tGC procedure name" << std::endl;
        of << "TOFDrift:\t" << instsetup_->TOFDrift() << "\tTOF correction factor" << std::endl;
        of << "samplesize:\t" << instsetup_->samplesize() << "\tSample size" << std::endl;
        of << "sampleunits:\t" << instsetup_->sampleunits() << "\tSample size units" << std::endl;
        of << "peakcentroid:\t" << instsetup_->peakcentroid() << "\tCentroiding method:\t" << instsetup_->describe_peakcentroid() << std::endl;
        of << "pkintensity:\t" << instsetup_->pkintensity() << "\tIntensity method (0 = height, 1 = area)" << std::endl;
        of << "inithreshold:\t" << instsetup_->inithreshold() << "\tThreshold at low mass (as A/D value)" << std::endl;
        of << "fnlthreshold:\t" << instsetup_->fnlthreshold() << "\tThreshold at high mass (as A/D value)" << std::endl;
        of << "HVolt:\t" << instsetup_->HVolt() << "\tAccelerating voltage" << std::endl;
        of << "HVscanBValue:\t" << instsetup_->HVscanBValue() << "\tCalibration intercept for HV scan" << std::endl;
        of << "Peakthres:\t" << instsetup_->Peakthres() << "\tCentroiding algorithm threshod(%)" << std::endl;
        of << "baseline:\t" << instsetup_->baseline() << "\tMeasured instrument baseline" << std::endl;
        of << "noise:\t" << instsetup_->noise() << "\tMeassured instrument baseline noise" << std::endl;
        of << "linkcorrection:\t" << instsetup_->linkcorrection() << "\tMass correction for linked scans" << std::endl;
        of << "valley:\t" << instsetup_->valley() << "\tCentroiding algorithm valle(%)" << std::endl;
        of << "minpeakwidth:\t" << instsetup_->minpeakwidth() << "\tCentroiding algorithm minimum peak width(%)" << std::endl;
        of << "sampletype:\t" << instsetup_->sampletype() << "\t0:solid, 1:Solid by Dry Weight, 2: Liquid, 3: Gas" << std::endl;
        of << "unitscode:\t" << instsetup_->unitscode() << "\tSample size units( 0:ug, 3:Kg)" << std::endl;
        of << "dryweight:\t" << instsetup_->dryweight() << "\tPercent dry weight" << std::endl;
        of << "linkmass:\t" << instsetup_->linkmass() << "\tLink mass" << std::endl;
        of << "SIMfield:\t" << instsetup_->SIMfield() << "\tSwitching Field for SIM" << std::endl;
        of << "SIMBset:\t" << instsetup_->SIMBset() << "\tMagnet Field reference value used for EF SIM" << std::endl;
        of << "SIMBfield:\t" << instsetup_->SIMBfield() << "\tMagnet field value used for EF SIM" << std::endl;
        of << "Slitcouple:\t" << instsetup_->Slitcouple() << "\tSlits coupled?" << std::endl;
        of << "Maxmassrange:\t" << instsetup_->Maxmassrange() << "\tMaximum mass range for instrument" << std::endl;
        of << "HvscanBDrive:\t" << instsetup_->HvscanBDrive() << "\tMagnet field reference used of HV scan" << std::endl;
        of << "SIMCalOK:\t" << instsetup_->SIMCalOK() << "\tSIM calibration OK" << std::endl;
        of << "Maxvolt:\t" << instsetup_->Maxvolt() << "\tMaximum High voltage for instrument" << std::endl;
        of << "PeakFilter:\t" << instsetup_->PeakFilter() << "\t" << std::endl;
        of << "TwoWayScan:\t" << instsetup_->TwoWayScan() << "\t" << std::endl;
        of << "Dummy:\t" << instsetup_->Dummy() << "\tFuture use" << std::endl;
    }
    if ( lrpcalib_ && *lrpcalib_ ) {
        of << "flags:\t" << lrpcalib_->flags() << std::endl;
        for ( int i = 0; i < lrpcalib_->cal_size; ++i ) {
            auto& cal = lrpcalib_->cal()[i];
            of << "mass:\t" << double(cal.m) / 65536 << ", intensity:\t" << cal.i << ", coeffa: "
               << cal.coeffa << ", coeffb: " << cal.coeffb << std::endl;
        }
        of << "type:\t" << lrpcalib_->type() << std::endl;
    }

    if ( lrptic_ && *lrptic_ ) {
        of << "----------- TIC --------------";
        of << "nexttpr:\t" << lrptic_->nextptr() << std::endl;
        for ( auto& tic: lrptic_->tic() ) {
            of << boost::format( "\ttime: %.3fs\tintens: %d\toffs: 0x%x\toverload: %d\n" ) % (double( tic.time ) / 1000.0) % tic.intensity % tic.ptr % tic.overload;
        }
        of << "----------- END of TIC --------------";
    }
}

