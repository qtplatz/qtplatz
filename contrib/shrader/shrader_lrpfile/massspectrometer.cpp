/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "massspectrometer.hpp"
#include "constants.hpp"
#include "helper.hpp"
#include <instsetup.hpp>
#include <lrpcalib.hpp>
#include <lrpfile.hpp>
#include <lrphead2.hpp>
#include <lrphead3.hpp>
#include <lrpheader.hpp>
#include <lrptic.hpp>
#include <simions.hpp>
#include <adplugin/plugin.hpp>
#include <adplugin/visitor.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/lapfinder.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <adcontrols/datareader.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/polfit.hpp>
#include <adutils/datafile_signature.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <mutex>

namespace shrader {

    class MassSpectrometer::impl {
    public:
        lrpfile lrpfile_;
    };
}

using namespace shrader;

MassSpectrometer::~MassSpectrometer()
{
}

MassSpectrometer::MassSpectrometer() : impl_( std::make_unique< impl >() )
{
}

std::shared_ptr< adcontrols::ScanLaw >
MassSpectrometer::scanLaw( const adcontrols::MSProperty& prop ) const
{
    ADDEBUG() << "### MassSpectrometer::" << __FUNCTION__ << " ###";
    return {};
}

void
MassSpectrometer::setAcceleratorVoltage( double acclVoltage, double tDelay )
{
    ADDEBUG() << "### MassSpectrometer::" << __FUNCTION__ << " ###";
}

void
MassSpectrometer::initialSetup( adfs::sqlite& dbf, const boost::uuids::uuid& objuuid )
{
    // call from dataproc/dataprocessor.dpp etc.

    using namespace adutils::data_signature;

    adfs::stmt sql( dbf );

    std::map< std::string, value_t > sigs;
    sql >> sigs;
    if ( sigs.find( "lrpheader" ) != sigs.end() ) {
        impl_->lrpfile_.xload( lrpheader{}, string_to_block( std::get< std::string >( sigs["lrpheader"] ) ) );
    }
    if ( sigs.find( "lrpheader2" ) != sigs.end() ) {
        impl_->lrpfile_.xload( lrphead2{}, string_to_block( std::get< std::string >( sigs["lrpheader2"] ) ) );
    }
    if ( sigs.find( "lrpheader3" ) != sigs.end() ) {
        impl_->lrpfile_.xload( lrphead3{}, string_to_block( std::get< std::string >( sigs["lrpheader3"] ) ) );
    }
    if ( sigs.find( "instsetup" ) != sigs.end() ) {
        impl_->lrpfile_.xload( instsetup{}, string_to_block( std::get< std::string >( sigs["instsetup"] ) ) );
    }
    if ( sigs.find( "calib" ) != sigs.end() ) {
        impl_->lrpfile_.xload( lrpcalib{}, string_to_block( std::get< std::string >( sigs["calib"] ) ) );
    }
    // ADDEBUG() << "### MassSpectrometer::" << __FUNCTION__ << " ### "
    //           << "mass range: " << std::make_tuple(impl_->lrpfile_.instsetup().lmasslim()
    //                                                , impl_->lrpfile_.instsetup().umasslim() );
}

bool
MassSpectrometer::assignMasses( adcontrols::MassSpectrum& ms, int64_t rowid ) const
{
    auto mass_range = std::make_pair( double(impl_->lrpfile_.instsetup().lmasslim())/65536.0
                                      , double( impl_->lrpfile_.instsetup().umasslim())/65536.0 );

    // check shrader::massspectrometer class -- also has same code
    auto mass_at = [&]( size_t idx )->double{
        // assume time squared scan law (TOF Eq.)
        const double f = double( idx ) / double( ms.size() - 1 );
        const double s0 = std::sqrt( mass_range.first );
        const double s1 = std::sqrt( mass_range.second );
        const double s = s0 + f * ( s1 - s0 );
        return s * s;
    };
    for ( size_t i = 0; i < ms.size(); ++i )
        ms.setMass( i, mass_at( i ) );
    ADDEBUG() << "### MassSpectrometer::" << __FUNCTION__ << " ### -- masses resetted --";
    return true;
}

double
MassSpectrometer::assignMass( double time, int mode ) const
{
    ADDEBUG() << "### MassSpectrometer::" << __FUNCTION__ << " ###";
    return 0;
}

const char * const
MassSpectrometer::massSpectrometerName() const
{
    using namespace shreader::spectrometer;
    return names::objtext_massspectrometer;
}

const boost::uuids::uuid&
MassSpectrometer:: massSpectrometerClsid() const
{
    using namespace shreader::spectrometer;
    return iids::uuid_massspectrometer;
}

const adcontrols::ScanLaw *
MassSpectrometer::scanLaw() const
{
    return {};
}

const char *
MassSpectrometer::dataInterpreterText() const
{
    using namespace shreader::spectrometer;
    return names::objtext_datainterpreter;
}

const boost::uuids::uuid&
MassSpectrometer::dataInterpreterUuid() const
{
    using namespace shreader::spectrometer;
    return iids::uuid_datainterpreter;
}
