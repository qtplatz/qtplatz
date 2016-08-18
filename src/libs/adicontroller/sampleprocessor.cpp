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

using namespace adicontroller;

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
        storage_name_.replace_extension( ".adfs" );

        boost::system::error_code ec;
        boost::filesystem::rename( progress_name, storage_name_, ec );
        if ( ec ) 
            ADDEBUG() << boost::format( "Sample %1% close failed: %2%" ) % storage_name_.stem().string() % ec.message();
        else 
            ADTRACE() << boost::format( "Sample %1% closed." ) % storage_name_.stem().string();
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
SampleProcessor::prepare_storage( adicontroller::SignalObserver::Observer * masterObserver )
{
    masterObserver_ = masterObserver->shared_from_this();
    
    boost::filesystem::path path( sampleRun_->dataDirectory() );

	if ( ! boost::filesystem::exists( path ) )
		boost::filesystem::create_directories( path );

    auto pair = sampleRun_->findNextRunName();

	boost::filesystem::path filename = path / pair.first;
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

    boost::filesystem::path prefix = adportable::split_filename::prefix<wchar_t>( run.filePrefix() );

    int runno = 0;
	if ( boost::filesystem::exists( path ) && boost::filesystem::is_directory( path ) ) {
        using boost::filesystem::directory_iterator;
		for ( directory_iterator it( path ); it != directory_iterator(); ++it ) {
            boost::filesystem::path fname = (*it);
			if ( fname.extension().string() == ".adfs" ) {
                runno = std::max( runno, adportable::split_filename::trailer_number_int( fname.stem().wstring() ) );
            }
        }
    }
    std::wostringstream o;
    o << prefix.wstring() << std::setw( 4 ) << std::setfill( L'0' ) << (runno + 1);
    
    boost::filesystem::path filename = path / o.str();
	filename.replace_extension( ".adfs~" );

    run.setFilePrefix( filename.stem().wstring() );

    return filename;
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
SampleProcessor::handle_data( unsigned long objId, long pos, const SignalObserver::DataReadBuffer& rdBuf )
{
    if ( rdBuf.events() & SignalObserver::wkEvent_INJECT ) {
        ts_inject_trigger_ = rdBuf.epoch_time();
        tp_inject_trigger_ = std::chrono::steady_clock::now(); // CAUTION: this has some unknown delay from exact trigger.
        if ( !c_acquisition_active_ )
            ADTRACE() << boost::format( "Sample '%1%' got an INJECTION" ) % this->storage_name_.string();
		c_acquisition_active_ = true;
    }

	if ( ! c_acquisition_active_ )
		return;

	adfs::stmt sql( fs_->db() );
	sql.prepare( "INSERT INTO AcquiredData VALUES( :oid, :time, :npos, :fcn, :events, :data, :meta )" );
	sql.begin();
	sql.bind( 1 ) = objId;
    sql.bind( 2 ) = rdBuf.timepoint();
	sql.bind( 3 ) = pos;
	sql.bind( 4 ) = rdBuf.fcn();
    sql.bind( 5 ) = rdBuf.events();
	sql.bind( 6 ) = adfs::blob( rdBuf.xdata().size(), reinterpret_cast<const int8_t *>( rdBuf.xdata().data() ) );
    sql.bind( 7 ) = adfs::blob( rdBuf.xmeta().size(), reinterpret_cast<const int8_t *>( rdBuf.xmeta().data() ) ); // method
	if ( sql.step() == adfs::sqlite_done )
		sql.commit();
	else
		sql.reset();

    if ( objId == objId_front_ && pos_front_ > unsigned( pos + 10 ) ) {
        size_t nBehind = pos_front_ - pos;
        if ( stop_triggered_ && ( nBehind % 5 == 0 ) )
            ADTRACE() << boost::format( "'%1%' is being closed.  Saving %2% more data in background." ) % storage_name_.stem() % nBehind;
        else if ( nBehind % 25 == 0 )
            ADTRACE() << boost::format( "Sample '%1%' %2% data behind." ) % storage_name_.stem() % nBehind;
    }

    auto elapsed_count = rdBuf.timepoint() - ts_inject_trigger_;
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - tp_inject_trigger_);
#if 0
    if ( objId == 1 )
        Logging( L"Elapsed time: %1%", EventLog::pri_INFO ) % double( elapsed_time.count() / 60.0 );
#endif
    if ( elapsed_time.count() >= sampleRun_->methodTime() || double( elapsed_count ) * 1.0e-6 >= sampleRun_->methodTime() ) {
        c_acquisition_active_ = false;
    }

}


void
SampleProcessor::write( const boost::uuids::uuid& objId
                        , SignalObserver::DataWriter& writer )
{
    writer.rewind();
    do {

        if ( ! c_acquisition_active_ ) {

            if ( writer.events() & SignalObserver::wkEvent_INJECT ) {
                ADDEBUG() << boost::format( "SampleProcessor INJECT TRIGGERD by DATA 0x%x %s" ) % writer.events() % boost::lexical_cast<std::string>( objId );
                if ( !c_acquisition_active_ ) { // protect from chattering
                    ts_inject_trigger_ = writer.epoch_time(); // uptime;
                    c_acquisition_active_ = true;
                }
            }
        }

        if ( c_acquisition_active_ ) {

            std::string xdata, xmeta;
            writer.xdata( xdata );
            writer.xmeta( xmeta );
            adutils::v3::AcquiredData::insert( fs_->db(), objId
                                               , writer.elapsed_time()
                                               , writer.epoch_time()
                                               , writer.pos()
                                               , writer.fcn()
                                               , writer.ndata()
                                               , writer.events()
                                               , xdata
                                               , xmeta );
        }
        
    } while( writer.next() );

}

void
SampleProcessor::populate_calibration( SignalObserver::Observer * parent )
{
    auto vec = parent->siblings();

    for ( auto observer : vec ) {
        std::string dataClass;
        octet_array data;
        int32_t idx = 0;
        while ( observer->readCalibration( idx++, data, dataClass ) ) {
            adfs::stmt sql( fs_->db() );
            sql.prepare( "INSERT INTO Calibration VALUES(:objuuid,:dataClass,:data,0)" );
            sql.bind( 1 ) = observer->objid();
            sql.bind( 2 ) = dataClass;
            sql.bind( 3 ) = adfs::blob( data.size(), reinterpret_cast<const int8_t *>( data.data() ) );
            if ( sql.step() == adfs::sqlite_done )
                sql.commit();
            else
                sql.reset();
        }
        populate_calibration( observer.get() );
    }
    
}

void
SampleProcessor::populate_descriptions( SignalObserver::Observer * parent )
{
    auto vec = parent->siblings();

    for ( auto observer : vec ) {

        if ( auto clsid = observer->dataInterpreterClsid() ) {
            (void)clsid;

            //const auto& a = observer->objid();
            //const auto& b = observer->description().data().objid;

            if ( observer->objid() != observer->description().data().objid ) {
                assert( observer->objid() == observer->description().data().objid );
                return;
            }

            adutils::v3::AcquiredConf::insert( fs_->db()
                                               , observer->objid()
                                               , observer->description().data() );
        }
        populate_descriptions( observer.get() );
    }
}

std::shared_ptr< const adcontrols::SampleRun >
SampleProcessor::sampleRun() const
{
    return sampleRun_;
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
