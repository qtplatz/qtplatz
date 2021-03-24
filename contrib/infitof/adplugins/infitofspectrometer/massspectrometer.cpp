/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC
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
#include <admtcontrols/scanlaw.hpp>
#include <admtcontrols/infitof.hpp>
#include <admtcontrols/orbitprotocol.hpp>
#include <infitofcontrols/constants.hpp>
#include <infitofcontrols/method.hpp>
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
#include "constants.hpp"
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

using namespace infitofspectrometer;

MassSpectrometer::~MassSpectrometer()
{
}

MassSpectrometer::MassSpectrometer() : scanLaw_( std::make_unique< admtcontrols::infitof::ScanLaw >() )
{
}

const wchar_t *
MassSpectrometer::name() const
{
    return infitofspectrometer::constants::dataInterpreter::spectrometer::name(); // "InfiTOF
}

std::shared_ptr< adcontrols::ScanLaw >
MassSpectrometer::scanLaw( const adcontrols::MSProperty& prop ) const
{
    if ( scanLaw_ ) {
        auto ptr = std::make_shared< admtcontrols::ScanLaw >( *scanLaw_ );
        ptr->setAcceleratorVoltage( prop.acceleratorVoltage() );
        ptr->setTDelay( prop.tDelay() );
        return ptr;
    } else {
        return std::make_shared< admtcontrols::infitof::ScanLaw >( prop.acceleratorVoltage(), prop.tDelay() );
    }
}

void
MassSpectrometer::setAcceleratorVoltage( double acclVoltage, double tDelay )
{
    if ( !scanLaw_ ) {
        scanLaw_ = std::make_unique< admtcontrols::infitof::ScanLaw >( acclVoltage, tDelay );
    } else {
        scanLaw_->setAcceleratorVoltage( acclVoltage );
        scanLaw_->setTDelay( tDelay );
    }
}

const char * const
MassSpectrometer::massSpectrometerName() const
{
    return ::infitof::names::objtext_massspectrometer; // 'InfiTOF'
}

const boost::uuids::uuid&
MassSpectrometer::massSpectrometerClsid() const
{
    static boost::uuids::uuid uuid = ::infitof::iids::uuid_massspectrometer;  // boost::uuids::string_generator()( clsid_text );
    return uuid;
}

const adcontrols::ScanLaw *
MassSpectrometer::scanLaw() const
{
    return scanLaw_.get();
}

void
MassSpectrometer::setMethod( const adcontrols::ControlMethod::Method& m )
{
    method_ = std::make_unique< adcontrols::ControlMethod::Method >( m );

    auto it = method_->find( method_->begin(), method_->end(), infitofcontrols::method::clsid() );
    if ( it != method_->end() ) {
        infitofcontrols::method im;
        if ( adcontrols::ControlMethod::MethodItem::get<>( *it, im ) ) {
            protocols_ = im.tof().protocols;
        }
    }
}

const adcontrols::ControlMethod::Method *
MassSpectrometer::method() const
{
    return method_.get();
}

int
MassSpectrometer::mode( uint32_t protocolNumber ) const
{
    if ( protocolNumber < protocols_.size() )
        return protocols_[ protocolNumber ].nlaps();
    return -1;
}

bool
MassSpectrometer::setMSProperty( adcontrols::MassSpectrum& ms, const adcontrols::ControlMethod::Method& m, int proto ) const
{
    auto it = m.find( m.begin(), m.end(), infitofcontrols::method::clsid() );
    if ( it != m.end() ) {
        infitofcontrols::method im;
        if ( adcontrols::ControlMethod::MethodItem::get<>( *it, im ) ) {
            if ( im.tof().protocols.size() > proto ) {
                ms.setAcquisitionMassRange( im.tof().protocols[proto].lower_mass, im.tof().protocols[proto].upper_mass );

                std::vector< adcontrols::TofProtocol > v;
                infitofcontrols::method::copy_protocols( im.tof(), v );
                if ( v.size() > proto ) {
                    auto& prop = ms.getMSProperty();
                    prop.setTofProtocol( v[ proto ] );
                }
                return true;
            }
        }
    }
    return false;
}

void
MassSpectrometer::initialSetup( adfs::sqlite& dbf, const boost::uuids::uuid& objuuid )
{
    double L1 = ::infitof::Constants::FLIGHT_LENGTH_L1;
    double L2 = ::infitof::Constants::FLIGHT_LENGTH_L2;
    double L3 = ::infitof::Constants::FLIGHT_LENGTH_L3;
    double LG = ::infitof::Constants::FLIGHT_LENGTH_LG;
    double L4 = ::infitof::Constants::FLIGHT_LENGTH_L4;
    double LT = ::infitof::Constants::FLIGHT_LENGTH_LT;
    double LE = ::infitof::Constants::FLIGHT_LENGTH_EXIT;

    adfs::stmt sql( dbf );

    sql.prepare( "SELECT acclVoltage,tDelay FROM ScanLaw WHERE objuuid=?" );
    sql.bind( 1 ) = objuuid;

    if ( sql.step() == adfs::sqlite_row ) {
        acceleratorVoltage_ = sql.get_column_value< double >( 0 );
        tDelay_             = sql.get_column_value< double >( 1 );
    }

    sql.prepare(
        "SELECT FLIGHT_LENGTH_L1"
        ", FLIGHT_LENGTH_L2"
        ", FLIGHT_LENGTH_L3"
        ", FLIGHT_LENGTH_LG"
        ", FLIGHT_LENGTH_L4"
        ", FLIGHT_LENGTH_LT"
        ", FLIGHT_LENGTH_EXIT"
        " FROM MULTUM_ANALYZER_CONFIG LIMIT 1");

    if ( sql.step() == adfs::sqlite_row ) {
        int row(0);
        L1 = sql.get_column_value<double>(row++);
        L2 = sql.get_column_value<double>(row++);
        L3 = sql.get_column_value<double>(row++);
        LG = sql.get_column_value<double>(row++);
        L4 = sql.get_column_value<double>(row++);
        LT = sql.get_column_value<double>(row++);
        LE = sql.get_column_value<double>(row++);
    }

    scanLaw_ = std::make_unique< admtcontrols::ScanLaw >( acceleratorVoltage_
                                                            , tDelay_
                                                            , L1, L2, L3, LG, L4, LT, LE );

    do {
        sql.prepare( "SELECT data from MetaData WHERE clsid = ?" );
        sql.bind( 1 ) = adcontrols::ControlMethod::Method::clsid();
        if ( sql.step() == adfs::sqlite_row ) {
            auto blob = sql.get_column_value< adfs::blob >( 0 );
            boost::iostreams::basic_array_source< char > device( reinterpret_cast< const char * >(blob.data()), size_t( blob.size() ) );
            boost::iostreams::stream< boost::iostreams::basic_array_source< char > > strm( device );
            auto m = std::make_unique< adcontrols::ControlMethod::Method >();
            if ( adcontrols::ControlMethod::Method::restore( strm, *m ) ) {
                method_ = std::move( m );
                auto it = method_->find( method_->begin(), method_->end(), infitofcontrols::method::clsid() );
                if ( it != method_->end() ) {
                    infitofcontrols::method im;
                    if ( adcontrols::ControlMethod::MethodItem::get<>( *it, im ) ) {
                        protocols_ = im.tof().protocols;
                        ADDEBUG() << "----------------- protocols ------------------- " << protocols_.size();
                        for ( const auto& p: protocols_ ) {
                            ADDEBUG() << std::make_pair( p.formulae(), p.nlaps() );
                            // ADDEBUG() << p.avgr_delay;
                            // ADDEBUG() << std::make_pair( p.exit.delay, p.exit.width );
                            // ADDEBUG() << std::make_pair( p.exit2.delay, p.exit2.width );
                            // ADDEBUG() << std::make_pair( p.gate.at(0).delay, p.gate.at(0).width ) << ", " << p.gate.at(0).enable;
                            // ADDEBUG() << std::make_pair( p.gate.at(1).delay, p.gate.at(1).width ) << ", " << p.gate.at(1).enable;
                        }
                    }
                }
            }
        }
    } while (0);

    bool hasScanLawTimeCourse( false );
    do {
        sql.prepare( "SELECT name FROM sqlite_master WHERE type='table' AND name='ScanLawTimeCourse'" );
        hasScanLawTimeCourse = ( sql.step() == adfs::sqlite_row );
    } while ( 0 );

    if ( hasScanLawTimeCourse ) {

        sql.prepare( "SELECT rowid,tDelay,acclVoltage FROM ScanLawTimeCourse ORDER BY rowid" );

        while ( sql.step() == adfs::sqlite_row ) {
            scanLaws_.emplace_back( sql.get_column_value< int64_t >( 0 )
                                    , std::make_unique< admtcontrols::ScanLaw >( sql.get_column_value< double >( 2 )
                                                                                   , sql.get_column_value< double >( 1 )
                                                                                   , L1, L2, L3, LG, L4, LT, LE ) );
        }
    }
}

std::pair< int, double > // nlap, time
MassSpectrometer::findLaps( double mass, int proto ) const
{
    if ( !protocols_.empty() && protocols_.size() > proto ) {
        const auto& p = protocols_[ proto ];
        double t_mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( adcontrols::ChemicalFormula::split( p.formulae() ) ).first;
        adcontrols::lapFinder finder( *this->scanLaw_, t_mass, p.nlaps() );
        return finder( mass );
    }
    return {0,0};
}

bool
MassSpectrometer::estimateScanLaw( const adcontrols::MSPeaks& peaks, double& va, double& t0 ) const
{
    if ( auto law = scanLaw() ) {

        if ( peaks.size() == 1 ) {

            const adcontrols::MSPeak& pk = peaks[ 0 ];
            va = scanLaw_->acceleratorVoltage( pk.exact_mass(), pk.time(), pk.mode(), 0.0 );
            t0 = 0.0;
            ADDEBUG() << "scanlaw: " << va << ", " << t0;
            return true;

        } else if ( peaks.size() >= 2 ) {

            std::vector<double> x, y, coeffs;

            for ( auto& pk : peaks ) {
                x.push_back( std::sqrt( pk.exact_mass() ) * law->fLength( pk.mode() ) );
                y.push_back( pk.time() );
            }

            if ( adportable::polfit::fit( x.data(), y.data(), x.size(), 2, coeffs ) ) {

                t0 = coeffs[ 0 ];
                double t1 = adportable::polfit::estimate_y( coeffs, 1.0 ); // estimate tof for m/z = 1.0, 1mL
                va = adportable::TimeSquaredScanLaw::acceleratorVoltage( 1.0, t1, 1.0, t0 );

                return true;
            }
        }
    }
    return false;
}

const adcontrols::ScanLaw *
MassSpectrometer::scanLaw( int64_t rowid ) const
{
    if ( !scanLaws_.empty() && rowid >= 0 ) {
        auto it = std::lower_bound( scanLaws_.begin(), scanLaws_.end(), rowid, [](const auto& a, const int64_t& b){ return a.first < b; } );
        if ( it != scanLaws_.end() && it->second ) {
            // ADDEBUG() << "scanLaw(" << rowid << ") found " << it->first;
            return it->second.get();
        }
    }
    return scanLaw_.get();
}

bool
MassSpectrometer::assignMasses( adcontrols::MassSpectrum& ms, int64_t rowid ) const
{
    auto mode = ms.mode();
    (void)mode;
    auto scanlaw = scanLaw( rowid );
    return ms.assign_masses( [&]( double time, int mode ) { return scanlaw->getMass( time, mode ); } );
}

double
MassSpectrometer::assignMass( double time, int mode ) const
{
    if ( scanLaw_ )
        return scanLaw_->getMass( time, mode );
    return 0;
}

const char *
MassSpectrometer::dataInterpreterText() const
{
    return infitof::names::objtext_datainterpreter; // "IniTOF"
}

const boost::uuids::uuid&
MassSpectrometer::dataInterpreterUuid() const
{
    return infitof::iids::uuid_datainterpreter;
}
