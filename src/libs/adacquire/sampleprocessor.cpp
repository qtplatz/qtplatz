/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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
#include "task.hpp"
#include "mscalibio_v3.hpp"
#ifndef NDEBUG
#include "../../../contrib/agilent/libs/acqrscontrols/constants.hpp"
#endif
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
    if ( ! closed_flag_ ) {
        __close();
    }
}

SampleProcessor::SampleProcessor( std::shared_ptr< adcontrols::SampleRun > run
                                  , std::shared_ptr< adcontrols::ControlMethod::Method > cmth )
    : fs_( new adfs::filesystem )
    , c_acquisition_active_( false )
    , myId_( __nid__++ )
    , objId_front_( 0 )
    , pos_front_( 0 )
    , stop_triggered_( false )
    , sampleRun_( run )
    , ctrl_method_( cmth )
    , ts_inject_trigger_( 0 )
    , elapsed_time_( 0 )
    , closed_flag_( false )
    , deffered_count_( 0 )
{
}

void
SampleProcessor::close( bool detach )
{
    closed_flag_ = true;
    tp_close_trigger_ = std::chrono::steady_clock::now();
    if ( detach ) {
        auto self = this->shared_from_this();
        close_future_ = std::async(std::launch::async, [self,this]{
            __close();
        });
    } else {
        __close();
    }
}

void
SampleProcessor::__close()
{
    try {
        if ( auto observer = masterObserver_.lock() ) {
            observer->closingStorage( *this );
            observer.reset();
        }

        if ( thread_.joinable() ) {
            que_.emplace_back();
            sema_.signal();
            thread_.join();
        }

        fs_->close();

        std::filesystem::path progress_name = storage_name_;
        boost::system::error_code ec;

        if ( c_acquisition_active_ ) {
            storage_name_.replace_extension( ".adfs" ); // *.adfs~ -> *.adfs
            std::filesystem::rename( progress_name, storage_name_, ec );
            if ( ec )
                ADDEBUG() << boost::format( "Sample %1% close failed: %2%" ) % storage_name_.stem().string() % ec.message();
        } else {
            std::filesystem::remove( storage_name_, ec );
            if ( ec )
                ADDEBUG() << boost::format( "Sample %1% remove failed: %2%" ) % storage_name_.stem().string() % ec.message();
        }

        if ( close_future_ ) {
            boost::asio::post( task::instance()->io_service(), [&]{ close_future_->get(); } );
        }
        auto duration = std::chrono::duration< double >( std::chrono::steady_clock::now() - tp_close_trigger_).count();
        ADINFO() << boost::format( "SampleProcessor: %s\tclosed. Took %.1f seconds to complete." ) % storage_name_.stem().string() % duration;

    } catch ( std::exception& e ) {
        ADDEBUG() << boost::diagnostic_information( e );
    } catch ( ... ) {
        ADDEBUG() << boost::current_exception_diagnostic_information();
    }
}

void
SampleProcessor::prepare_storage( adacquire::SignalObserver::Observer * masterObserver )
{
    ADDEBUG() << "################### " << __FUNCTION__;
    masterObserver_ = masterObserver->shared_from_this();

    std::filesystem::path path( sampleRun_->dataDirectory() );

	if ( ! std::filesystem::exists( path ) )
		std::filesystem::create_directories( path );

	std::filesystem::path filename = sampleRun_->filename();
	filename.replace_extension( ".adfs~" );

    std::error_code ec;
    storage_name_ = std::filesystem::weakly_canonical( filename, ec );
    if ( ec ) {
        ADDEBUG() << "##### Error: " << __FUNCTION__ << "\t" << ec.message();
    }

    sampleRun_->setFilePrefix( filename.stem().wstring() );

	///////////// creating filesystem ///////////////////
    if ( !fs_->create( storage_name_.wstring().c_str(), 8192*8, 8192 ) ) {
        ADDEBUG() << "############### adfs creation failed: " << storage_name_;
        return;
    }

    adutils::v3::AcquiredConf::create_table_v3( fs_->db() );
    adutils::v3::AcquiredData::create_table_v3( fs_->db() );
    adacquire::v3::mscalibio::create_table_v3( fs_->db() );

	populate_descriptions( masterObserver );
    populate_calibration( masterObserver );

    masterObserver->prepareStorage( *this );

    thread_ = std::thread( [this]{ this->writer_thread(); } );
}

std::filesystem::path
SampleProcessor::prepare_sample_run( adcontrols::SampleRun& run, bool createDirectory )
{
    std::filesystem::path path( run.dataDirectory() );

    if ( !std::filesystem::exists( path ) ) {
        if ( !createDirectory )
            return std::filesystem::path();
        std::filesystem::create_directories( path );
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

const std::filesystem::path&
SampleProcessor::storage_name() const
{
    return storage_name_;
}


void
SampleProcessor::writer_thread()
{
    do {
        sema_.wait();

        boost::uuids::uuid objId;
        std::shared_ptr< SignalObserver::DataWriter > writer;
        do {
            std::lock_guard< std::mutex > lock( mutex_ );
            if ( que_.empty() )
                return; // end of thread

            std::tie( objId, writer ) = que_.front();
            que_.pop_front();

            if ( objId == boost::uuids::uuid{{0}} || writer == nullptr ) {
                if ( deffered_count_ ) {
                    task::instance()->handleDataWriterStatus( myId_, fs_->filename(), 0, deffered_count_, 0);
                }
                return; // end of thread
            }
        } while (0);

        __write( objId, writer );

        if ( c_acquisition_active_ && closed_flag_ ) {
            ++deffered_count_;
            auto duration = std::chrono::duration< double >( std::chrono::steady_clock::now() - tp_close_trigger_).count();
            task::instance()->handleDataWriterStatus( myId_, fs_->filename(), sema_.count(), deffered_count_, duration );
        }
    } while ( true );
}

void
SampleProcessor::write( const boost::uuids::uuid& objId
                        , std::shared_ptr< SignalObserver::DataWriter > writer )
{
    if ( closed_flag_ )
        ADDEBUG() << "----- add new writer while already be closed. flag: " << closed_flag_;

    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        que_.emplace_back( objId, writer );
    } while ( 0 );
    sema_.signal();
}

uint32_t
SampleProcessor::__write( const boost::uuids::uuid& objId
                          , std::shared_ptr< SignalObserver::DataWriter > writer )
{
    uint32_t wcount(0);

    writer->rewind();
#if !defined NDEBUG && 0
    if ( acqrscontrols::u5303a::timecount_observer == objId )
        if ( auto p = writer->accessor()->pos_range() )
            ADDEBUG() << "========================== writer rewind ===============: " << writer->myId()
                      << ", " << *p;
#endif
    do {

        if ( ! c_acquisition_active_ ) {

            if ( writer->events() & SignalObserver::wkEvent_INJECT ) {
                // ADINFO() << boost::format ( "INJECT TRIGGERD [%s] EVENT: 0x%x OBJECT: " ) % fs_->filename() % writer->events() << objId;
                if ( !c_acquisition_active_ ) { // protect from chattering
                    ts_inject_trigger_ = writer->epoch_time(); // uptime;
                    c_acquisition_active_ = true;
                }
            }
        }

        if ( c_acquisition_active_ ) {
            wcount++;

            if ( ! writer->write ( *fs_, objId ) ) { // check if specific data writer implemented

                ADDEBUG() << "############## SampleProcessor DATA WRITE ERROR ############# " << wcount << ", pos=" << writer->pos();

            }
        }

    } while( writer->next() );

    return wcount;
}

void
SampleProcessor::populate_calibration( SignalObserver::Observer * parent )
{
    populate_calibration( parent, fs_->db() );
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
    // ADDEBUG() << "### workaround apply: SampleProcessor::set_inject_triggered(" << f << ")";
    // c_acquisition_active_ = f;
    // ADDEBUG() << "### end workaround";
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
        adacquire::v3::mscalibio::create_table_v3( db );

        populate_descriptions( masterObserver.get(), db );
        populate_calibration( masterObserver.get(), db );

        return true;
    }

    return false;
}
