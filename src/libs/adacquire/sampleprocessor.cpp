/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#include "sampleprocessor.hpp"
#include "datawriter.hpp"
#include "signalobserver.hpp"
#include "mscalibio_v3.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adfs/file.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/split_filename.hpp>
#include <adportable/utf.hpp>
#include <adutils/acquiredconf_v3.hpp>
#include <adutils/acquireddata_v3.hpp>
#include <boost/date_time.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <regex>
#include <string>
#include <cassert>

using namespace adacquire;

static size_t __nid__;

SampleProcessor::~SampleProcessor()
{
    try {
        if ( auto observer = masterObserver_.lock() ) {
            observer->closingStorage( *this );
            observer.reset();
        }
        
        fs_->close();

        boost::filesystem::path progress_name = storage_name_;
        boost::system::error_code ec;
        
        if ( c_acquisition_active_ ) {
            storage_name_.replace_extension( ".adfs" ); // *.adfs~ -> *.adfs
            boost::filesystem::rename( progress_name, storage_name_, ec );
            if ( ec ) 
                ADDEBUG() << boost::format( "Sample %1% close failed: %2%" ) % storage_name_.stem().string() % ec.message();
        } else {
            boost::filesystem::remove( storage_name_, ec );
            if ( ec )
                ADDEBUG() << boost::format( "Sample %1% remove failed: %2%" ) % storage_name_.stem().string() % ec.message();
        }
        
    } catch ( std::exception& e ) {
        ADDEBUG() << boost::diagnostic_information( e );
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
    }
}

SampleProcessor::SampleProcessor( std::shared_ptr< adcontrols::SampleRun > run
                                  , std::shared_ptr< adcontrols::ControlMethod::Method > cmth ) : fs_( new adfs::filesystem )
                                                                                                , c_acquisition_active_( false )
                                                                                                , myId_( __nid__++ )
                                                                                                , objId_front_( 0 )
                                                                                                , pos_front_( 0 )
                                                                                                , stop_triggered_( false )
                                                                                                , sampleRun_( run )
                                                                                                , ctrl_method_( cmth )
                                                                                                , ts_inject_trigger_( 0 )
                                                                                                , elapsed_time_( 0 )
{
}

void
SampleProcessor::prepare_storage( adacquire::SignalObserver::Observer * masterObserver )
{
    masterObserver_ = masterObserver->shared_from_this();
    
    boost::filesystem::path path( sampleRun_->dataDirectory() );

	if ( ! boost::filesystem::exists( path ) )
		boost::filesystem::create_directories( path );

	boost::filesystem::path filename = sampleRun_->filename();
	filename.replace_extension( ".adfs~" );

	storage_name_ = filename.normalize();

    sampleRun_->setFilePrefix( filename.stem().wstring() );
	
	///////////// creating filesystem ///////////////////
    if ( !fs_->create( storage_name_.wstring().c_str() ) )
        return;

    adutils::v3::AcquiredConf::create_table_v3( fs_->db() );
    adutils::v3::AcquiredData::create_table_v3( fs_->db() );
    v3::mscalibio::create_table_v3( fs_->db() );
	
	populate_descriptions( masterObserver );
    populate_calibration( masterObserver );

    masterObserver->prepareStorage( *this );
}

boost::filesystem::path
SampleProcessor::prepare_sample_run( adcontrols::SampleRun& run, bool createDirectory )
{
    boost::filesystem::path path( run.dataDirectory() );
    
    if ( !boost::filesystem::exists( path ) ) {
        if ( !createDirectory )
            return boost::filesystem::path();
        boost::filesystem::create_directories( path );
    }

    return run.filename( L".adfs~" );
}

void
SampleProcessor::stop_triggered()
{
    stop_triggered_ = true;
}

void
SampleProcessor::pos_front( unsigned int pos, unsigned long objId )
{
    if ( pos > pos_front_ ) {
        // keep largest pos and it's objId
        pos_front_ = pos;
        objId_front_ = objId;
    }
}

const boost::filesystem::path&
SampleProcessor::storage_name() const
{
    return storage_name_;
}

void
SampleProcessor::write( const boost::uuids::uuid& objId
                        , SignalObserver::DataWriter& writer )
{
    writer.rewind();
    do {

        if ( ! c_acquisition_active_ ) {

            if ( writer.events() & SignalObserver::wkEvent_INJECT ) {
                ADINFO() << boost::format ( "SampleProcessor [%s] INJECT TRIGGERD by EVENT 0x%x AT OBJECT " ) % fs_->filename() % writer.events() << objId;
                if ( !c_acquisition_active_ ) { // protect from chattering
                    ts_inject_trigger_ = writer.epoch_time(); // uptime;
                    c_acquisition_active_ = true;
                }
            }
        }

        if ( c_acquisition_active_ ) {
              
            if ( ! writer.write ( *fs_ ) ) {
                
                std::string xdata, xmeta;
                writer.xdata ( xdata );
                writer.xmeta ( xmeta );
                adutils::v3::AcquiredData::insert ( fs_->db(), objId
                                                    , writer.elapsed_time()
                                                    , writer.epoch_time()
                                                    , writer.pos()
                                                    , writer.fcn()
                                                    , writer.ndata()
                                                    , writer.events()
                                                    , xdata
                                                    , xmeta );
            }
        }
        
    } while( writer.next() );
}

void
SampleProcessor::populate_calibration( SignalObserver::Observer * parent )
{
    populate_calibration( parent, fs_->db() );

    // auto vec = parent->siblings();

    // for ( auto observer : vec ) {
    //     std::string dataClass;
    //     octet_array data;
    //     int32_t idx = 0;
    //     while ( observer->readCalibration( idx++, data, dataClass ) ) {
    //         adfs::stmt sql( fs_->db() );
    //         sql.prepare( "INSERT INTO Calibration VALUES(:objuuid,:dataClass,:data,0)" );
    //         sql.bind( 1 ) = observer->objid();
    //         sql.bind( 2 ) = dataClass;
    //         sql.bind( 3 ) = adfs::blob( data.size(), reinterpret_cast<const int8_t *>( data.data() ) );
    //         if ( sql.step() == adfs::sqlite_done )
    //             sql.commit();
    //         else
    //             sql.reset();
    //     }
    //     populate_calibration( observer.get() );
    // }
}

// static
void
SampleProcessor::populate_calibration( SignalObserver::Observer * parent, adfs::sqlite& db )
{
    auto vec = parent->siblings();

    for ( auto observer : vec ) {
        std::string dataClass;
        octet_array data;
        int32_t idx = 0;
        while ( observer->readCalibration( idx++, data, dataClass ) ) {
            adfs::stmt sql( db );
            sql.prepare( "INSERT INTO Calibration VALUES(:objuuid,:dataClass,:data,0)" );
            sql.bind( 1 ) = observer->objid();
            sql.bind( 2 ) = dataClass;
            sql.bind( 3 ) = adfs::blob( data.size(), reinterpret_cast<const int8_t *>( data.data() ) );
            if ( sql.step() == adfs::sqlite_done )
                sql.commit();
            else
                sql.reset();
        }
        populate_calibration( observer.get(), db );
    }
    
}

// static
void
SampleProcessor::populate_descriptions( SignalObserver::Observer * parent, adfs::sqlite& db )
{
    auto vec = parent->siblings();

    for ( auto observer : vec ) {

        if ( auto clsid = observer->dataInterpreterClsid() ) {
            (void)clsid;

            if ( observer->objid() != observer->description().data().objid ) {
                assert( observer->objid() == observer->description().data().objid );
                return;
            }

            adutils::v3::AcquiredConf::insert( db
                                               , observer->objid()
                                               , observer->description().data() );
        }
        populate_descriptions( observer.get(), db );
    }
}

void
SampleProcessor::populate_descriptions( SignalObserver::Observer * parent )
{
    populate_descriptions( parent, fs_->db() );

    // auto vec = parent->siblings();

    // for ( auto observer : vec ) {

    //     if ( auto clsid = observer->dataInterpreterClsid() ) {
    //         (void)clsid;

    //         //const auto& a = observer->objid();
    //         //const auto& b = observer->description().data().objid;

    //         if ( observer->objid() != observer->description().data().objid ) {
    //             assert( observer->objid() == observer->description().data().objid );
    //             return;
    //         }

    //         adutils::v3::AcquiredConf::insert( fs_->db()
    //                                            , observer->objid()
    //                                            , observer->description().data() );
    //     }
    //     populate_descriptions( observer.get() );
    // }
}

std::shared_ptr< const adcontrols::SampleRun >
SampleProcessor::sampleRun() const
{
    return sampleRun_;
}

std::shared_ptr< const adcontrols::ControlMethod::Method >
SampleProcessor::controlMethod() const
{
    return ctrl_method_;
}

bool
SampleProcessor::inject_triggered() const
{
    return c_acquisition_active_;
}

void
SampleProcessor::set_inject_triggered( bool f )
{
    // inProgress_ = f;
}

const uint64_t&
SampleProcessor::elapsed_time() const
{
    return elapsed_time_;
}

adfs::filesystem&
SampleProcessor::filesystem() const
{
    return *fs_;
}

bool
SampleProcessor::prepare_snapshot_storage( adfs::sqlite& db ) const
{
    if ( auto masterObserver = masterObserver_.lock() ) {

        adutils::v3::AcquiredConf::create_table_v3( db );
        adutils::v3::AcquiredData::create_table_v3( db );
        v3::mscalibio::create_table_v3( db );
	
        populate_descriptions( masterObserver.get(), db );
        populate_calibration( masterObserver.get(), db );

        return true;
    }

    return false;
}
