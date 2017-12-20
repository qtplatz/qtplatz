/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "document.hpp"
#include "constants.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/msmoltable.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adacquire/sampleprocessor.hpp>
#include <adportable/debug.hpp>
#include <adportable/binary_serializer.hpp>
#include <adspectrometer/massspectrometer.hpp>
#include <multumcontrols/scanlaw.hpp>
#include <QVariant>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

Q_DECLARE_METATYPE( boost::uuids::uuid );

using namespace adacquire;

document::document()
{
    // diagnostic
    QVariant v;
    v.setValue( boost::uuids::uuid() );
}

document::~document()
{
}

void
document::initialSetup()
{
}

void
document::finalClose()
{
}

bool
document::initStorage( const boost::uuids::uuid& uuid, adfs::sqlite& db ) const
{
    std::string objtext;

    if ( uuid == boost::uuids::uuid{ 0 } ) {
        objtext = "master.observer";
    } else {
        // auto it = observers_.find( uuid );
        // if ( it != observers_.end() )
        //     objtext = it->second->objtext();
        // else
        return false;
    }

#if ! defined NDEBUG && 0
    ADDEBUG() << "## " << __FUNCTION__ << " " << uuid << ", " << objtext;
#endif
    
    do {
        adfs::stmt sql( db );

        sql.exec( "CREATE TABLE IF NOT EXISTS MULTUM_ANALYZER_CONFIG ( \
 AnalyzerName TEXT PRIMARY KEY \
, SerialNumber TEXT \
, FLIGHT_LENGTH_L1 REAL \
, FLIGHT_LENGTH_L2 REAL \
, FLIGHT_LENGTH_L3 REAL \
, FLIGHT_LENGTH_LG REAL \
, FLIGHT_LENGTH_L4 REAL \
, FLIGHT_LENGTH_LT REAL \
, FLIGHT_LENGTH_EXIT REAL \
)" );

        sql.prepare( "INSERT OR REPLACE INTO MULTUM_ANALYZER_CONFIG (\
AnalyzerName\
, SerialNumber \
, FLIGHT_LENGTH_L1 \
, FLIGHT_LENGTH_L2 \
, FLIGHT_LENGTH_L3 \
, FLIGHT_LENGTH_LG \
, FLIGHT_LENGTH_L4 \
, FLIGHT_LENGTH_LT \
, FLIGHT_LENGTH_EXIT ) VALUES ( ?,?,?,?,?,?,?,?,? )" );

        sql.bind( 1 ) = std::string( adspectrometer::MassSpectrometer::class_name );
        sql.bind( 2 ) = std::string( "not-specified" );
        sql.bind( 3 ) = multumcontrols::infitof::FLIGHT_LENGTH_L1;
        sql.bind( 4 ) = multumcontrols::infitof::FLIGHT_LENGTH_L2;
        sql.bind( 5 ) = multumcontrols::infitof::FLIGHT_LENGTH_L3;
        sql.bind( 6 ) = multumcontrols::infitof::FLIGHT_LENGTH_LG;
        sql.bind( 7 ) = multumcontrols::infitof::FLIGHT_LENGTH_L4;
        sql.bind( 8 ) = multumcontrols::infitof::FLIGHT_LENGTH_LT;
        sql.bind( 9 ) = multumcontrols::infitof::FLIGHT_LENGTH_EXIT;

        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "sqlite error";
    } while ( 0 );

    static boost::uuids::uuid uuid_massspectrometer = boost::uuids::string_generator()( adspectrometer::MassSpectrometer::clsid_text ); 
#if 0
    if ( auto scanLaw = document::instance()->scanLaw() ) {
        
        adfs::stmt sql( db );
        sql.prepare( "\
INSERT OR REPLACE INTO ScanLaw (\
 objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer) \
 VALUES ( ?,?,?,?,?,? )" );
        sql.bind( 1 ) = uuid;
        sql.bind( 2 ) = objtext;
        sql.bind( 3 ) = scanLaw->kAcceleratorVoltage();
        sql.bind( 4 ) = scanLaw->tDelay();
        sql.bind( 5 ) = std::string( infitofspectrometer::MassSpectrometer::class_name );
        sql.bind( 6 ) = uuid_massspectrometer;

        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "sqlite error";

        sql.prepare( "INSERT OR REPLACE INTO Spectrometer ( id, scanType, description, fLength ) VALUES ( ?,?,?,? )" );
        sql.bind( 1 ) = uuid_massspectrometer;
        sql.bind( 2 ) = 0;
        sql.bind( 3 ) = std::string( "InfiTOF" );
        sql.bind( 4 ) = scanLaw->fLength( 0 ); // fLength at mode 0

        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "sqlite error";
    }
#endif
    // Save method
    if ( uuid == boost::uuids::uuid{ 0 } ) {
        // only if call for master observer
        
        adfs::stmt sql( db );
        sql.exec( "CREATE TABLE IF NOT EXISTS MetaData (clsid UUID, attrib TEXT, data BLOB )" ); // check adutils/AcquiredData::create_table_v3 
        
        std::string ar;
        {
            auto cm( cm_ );
            boost::iostreams::back_insert_device< std::string > inserter( ar );
            boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
            adcontrols::ControlMethod::Method::archive( device, *cm );
        }
        
        sql.prepare( "INSERT OR REPLACE INTO MetaData ( clsid, attrib, data ) VALUES ( ?,?,? )" );
        sql.bind( 1 ) = adcontrols::ControlMethod::Method::clsid();
        sql.bind( 2 ) = std::string( "ControlMethod::Method" );
        sql.bind( 3 ) = adfs::blob( ar.size(), reinterpret_cast< const int8_t * >( ar.data() ) );
        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "sqlite error";
    };

    return true;
}

bool
document::prepareStorage( const boost::uuids::uuid& uuid, adacquire::SampleProcessor& sp ) const
{
    // document::instance()->progress( 0.0, sp.sampleRun() ); // show data name on top of waveformwnd

    if ( initStorage( uuid, sp.filesystem().db() ) && uuid == boost::uuids::uuid{ 0 } ) {
        
        // counting peaks
        if ( uuid == boost::uuids::uuid{ 0 } ) {
            
            adfs::stmt sql( sp.filesystem().db() );
            
            sql.exec(
                "CREATE TABLE "
                "trigger ("
                " id INTEGER PRIMARY KEY"
                ", protocol INTEGER"
                ", timeSinceEpoch INTEGER"
                ", elapsedTime REAL"
                ", events INTEGER"
                ", threshold REAL"
                ", algo INTEGER )" );
            
            sql.exec(
                "CREATE TABLE "
                "peak ("
                " idTrigger INTEGER"
                ", peak_time REAL"
                ", peak_intensity REAL"
                ", front_offset INTEGER"
                ", front_intensity REAL"
                ", back_offset INTEGER"
                ", back_intensity REAL"
                ", FOREIGN KEY( idTrigger ) REFERENCES trigger( id ))" );
        };
        
        return true;
    }

    return false;
}

bool
document::closingStorage( const boost::uuids::uuid& uuid, adacquire::SampleProcessor& sp ) const
{
    if ( uuid == boost::uuids::uuid{ 0 } ) {

        if ( molTable_ ) {
            // Save ScanLaw lookup table
            adfs::stmt sql( sp.filesystem().db() );
            std::string ar;
            {
                boost::iostreams::back_insert_device< std::string > inserter( ar );
                boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
                adportable::binary::serialize<>()( *molTable_, ar );
            }
            
            sql.prepare( "INSERT OR REPLACE INTO MetaData ( clsid, attrib, data ) VALUES ( ?,?,? )" );
            sql.bind( 1 ) = adcontrols::MSMolTable::clsid();
            sql.bind( 2 ) = std::wstring( adcontrols::MSMolTable::dataClass() ); // 'MSMolTable'
            sql.bind( 3 ) = adfs::blob( ar.size(), reinterpret_cast< const int8_t * >( ar.data() ) );
            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << "sqlite error";
        }

        auto& fs = sp.filesystem();

        // Get longterm histogram
#if 0
        auto scanLaw = document::instance()->scanLaw();

        auto mass_assignee = [&] ( double time, int mode ) { return scanLaw->getMass( time, mode ); };
        using acqrscontrols::u5303a::tdcdoc;
        
        std::chrono::system_clock::time_point tp = std::chrono::system_clock::now(); 
        std::string date = adportable::date_string::logformat( tp );

        uint32_t serialnumber(0);
        auto waveform_observer = activeDigitizer_ == Digitizer::DigitizerU5303A ? acqrscontrols::u5303a::waveform_observer : acqrscontrols::ap240::waveform_observer;
        auto softavgr_observer = activeDigitizer_ == Digitizer::DigitizerU5303A ? acqrscontrols::u5303a::softavgr_observer : acqrscontrols::ap240::softavgr_observer;
    
        // get waveform(s)
        for ( auto& pair: { std::make_pair( spectra_[ waveform_observer ], "")
                    , std::make_pair( spectra_[ softavgr_observer ], "averaged" ) } ) {

            auto spectra = pair.first;
            int ch = 1;
            for ( auto& ms: spectra ) {
                if ( ms ) {
                    // set scanLaw
                    ms->getMSProperty().setAcceleratorVoltage( scanLaw_->kAcceleratorVoltage() );
                    ms->getMSProperty().setTDelay( scanLaw_->tDelay() );
                    
                    serialnumber = ms->getMSProperty().trigNumber();
                    QString title = QString( "Spectrum %1 CH-%2" ).arg( QString::fromStdString( date ), QString::number( ch ) );
                    QString folderId;
                    appendOnFile( fs, title, *ms, folderId );
                }
                ++ch;
            }
        }
#endif
#if 0
        // periodic histogram
        auto hgrm_0 = tdcdoc_->recentSpectrum( tdcdoc::PeriodicHistogram, mass_assignee );
        // long term histogram
        auto hgrm_1 = tdcdoc_->recentSpectrum( tdcdoc::LongTermHistogram, mass_assignee );
        // save histograms
        for ( auto pair: { std::make_pair( hgrm_0, "(periodic)" ) , std::make_pair( hgrm_1, "") } ) {
            int ch = 1;
            if ( pair.first ) {
                // set scanLaw
                pair.first->getMSProperty().setAcceleratorVoltage( scanLaw_->kAcceleratorVoltage() );
                pair.first->getMSProperty().setTDelay( scanLaw_->tDelay() );
                
                QString title = QString( "Histogram%1 %2 CH-%3" ).arg( pair.second, QString::fromStdString( date ), QString::number( ch ) );
                QString folderId;
                document::appendOnFile( fs, title, *pair.first, folderId );
            }
        }
#endif
    }
    return true;
}

void
document::setScanLawLookupTable( std::shared_ptr< const adcontrols::MSMolTable > t )
{
    molTable_ = t;
}

std::shared_ptr< const adcontrols::MSMolTable >
document::scanLawLookupTable() const
{
    return molTable_;
}

#if 0
std::shared_ptr< const adcontrols::MassSpectrometer >
document::massSpectrometer() const
{
    return impl_->massSpectrometer_;
}
#endif
