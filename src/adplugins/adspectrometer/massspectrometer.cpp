/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "massspectrometer.hpp"
#include "importdata.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/serializer.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adlog/logger.hpp>
#include <boost/exception/all.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <memory>

namespace adspectrometer {
    class MassSpectrometerException : public boost::exception, public std::exception {};

#if __APPLE__ || __linux__
    constexpr const char * MassSpectrometer::clsid_text;
    constexpr const char * MassSpectrometer::class_name;
#endif
}

namespace adspectrometer {

    class ScanLaw : public adcontrols::ScanLaw {
        adportable::TimeSquaredScanLaw t_;
    public:
        ScanLaw( double kAcceleratorVoltage, double tDelay, double fLength )
            : t_( kAcceleratorVoltage, tDelay, fLength ) {
        }
        double getMass( double secs, int mode ) const override {
            return t_.getMass( secs, mode );
        }
        double getTime( double mass, int mode ) const override {
            return t_.getTime( mass, mode );
        }
        double getMass( double secs, double fLength ) const override {
            return t_.getMass( secs, fLength );
        }
        double getTime( double mass, double fLength ) const override {
            return t_.getTime( mass, fLength );
        }
        double fLength( int mode ) const override {
            return t_.fLength( mode );
        }
    };
}

using namespace adspectrometer;

MassSpectrometer::~MassSpectrometer(void)
{
}

MassSpectrometer::MassSpectrometer( adcontrols::datafile * datafile )
    : adcontrols::MassSpectrometer( datafile )
    , accessor_( 0 )
    , scanlaw_( std::make_unique< ScanLaw >( acceleratorVoltage_, tDelay_, fLength_ ) )
{
}

void
MassSpectrometer::setAcceleratorVoltage( double acclVolts, double tDelay )
{
    acceleratorVoltage_ = acclVolts;
    tDelay_ = tDelay;
    scanlaw_ = std::make_unique< ScanLaw >( acceleratorVoltage_, tDelay_, fLength_ );
}

void
MassSpectrometer::initialSetup( adfs::sqlite& dbf, const boost::uuids::uuid& objuuid )
{
    adfs::stmt sql( dbf );
    
    boost::uuids::uuid clsidSpectrometer{ 0 };
    sql.prepare( "SELECT acclVoltage,tDelay,fLength FROM ScanLaw,Spectrometer"
                 " WHERE id=clsidSpectrometer"
                 " AND objuuid=?"
        );
    sql.bind( 1 ) = objuuid;
    if ( sql.step() == adfs::sqlite_row ) {
        acceleratorVoltage_ = sql.get_column_value< double >( 0 );
        tDelay_             = sql.get_column_value< double >( 1 );
        fLength_            = sql.get_column_value< double >( 2 );
    }
}

// void
// MassSpectrometer::setScanLaw( double acclVolts, double tDelay, double fLength )
// {
//     acceleratorVoltage_ = acclVolts;
//     tDelay_ = tDelay;
//     fLength_ = fLength;
//     scanlaw_ = std::make_unique< ScanLaw >( acceleratorVoltage_, tDelay_, fLength_ );
// }

bool
MassSpectrometer::subscribe( const adcontrols::LCMSDataset& data )
{
    accessor_ = &data;
    return true;
}
        
const wchar_t * 
MassSpectrometer::name() const 
{
    return L"adspectrometer::import";
}

const adcontrols::ScanLaw *
MassSpectrometer::scanLaw() const 
{
    return scanlaw_.get();
}

std::shared_ptr<adcontrols::ScanLaw>
MassSpectrometer::scanLaw( const adcontrols::MSProperty& ) const
{
    return std::make_shared< adspectrometer::ScanLaw >( 5000.0, 0.0, 1.0 );
}

void
MassSpectrometer::setCalibration( int mode, const adcontrols::MSCalibrateResult& )
{
    (void)mode;
}

const std::shared_ptr< adcontrols::MSCalibrateResult >
MassSpectrometer::getCalibrateResult( size_t idx ) const 
{
    (void)idx;
    return 0;
}

const adcontrols::MSCalibration *
MassSpectrometer::findCalibration( int mode ) const 
{
    (void)mode;
    return 0;
}

const import_continuum_massarray& 
MassSpectrometer::continuum_massarray() const
{
    if ( !continuum_massarray_ )
        const_cast< MassSpectrometer *>(this)->load_continuum_massarray();
    return *continuum_massarray_;
}

bool
MassSpectrometer::load_continuum_massarray()
{
    if ( !accessor_ && datafile_ )
        datafile_->accept( *this );

    if ( accessor_ ) {
        continuum_massarray_ = std::make_shared< import_continuum_massarray >();

        uint32_t objid = accessor_->findObjId( L"MS.PROFILE" );
        if ( objid > 0 ) {
            uint64_t fcn;
            std::vector<char> data, meta;
            if ( accessor_->getRaw( objid, 0, fcn, data, meta ) ) {
                if ( !meta.empty() ) {
                    if ( adportable::bzip2::is_a( meta.data(), meta.size() ) ) {
                        ADTRACE() << "load continuum massarray w/ decompress";
                        std::string ar;
                        adportable::bzip2::decompress( ar, meta.data(), meta.size() );
                        try {
                            ADTRACE() << "deserialize continuum massarray size=" << ar.size();
                            adportable::serializer< import_continuum_massarray >::deserialize( *continuum_massarray_, ar.data(), ar.size() );
                        } catch ( boost::archive::archive_exception& ex ) {
                            ADTRACE() << "archive_exception::code : " << ex.code << " " << ex.what();
                            BOOST_THROW_EXCEPTION( ex );
                        }
                    } else {
                        ADTRACE() << "load continuum massarray w/o decompress";
                        try {
                            adportable::serializer< import_continuum_massarray >::deserialize( *continuum_massarray_, meta.data(), meta.size() );
                        } catch ( boost::archive::archive_exception& ex ) {
                            ADWARN() << boost::current_exception_diagnostic_information() << ex.code;
                            BOOST_THROW_EXCEPTION( ex );
                        }
                    }
                    return true;
                }
            }
        }
    }
    return false;
}


const boost::uuids::uuid&
MassSpectrometer::objclsid() const
{
    static boost::uuids::uuid uuid = boost::uuids::string_generator()( clsid_text );
    return uuid;
}
