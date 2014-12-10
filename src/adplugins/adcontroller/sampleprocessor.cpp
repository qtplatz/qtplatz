/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include "task.hpp"
#include "logging.hpp"
#include <adcontrols/samplerun.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adinterface/signalobserverC.h>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/date_string.hpp>
#include <adlog/logger.hpp>
#include <adportable/profile.hpp>
#include <adutils/mscalibio.hpp>
#include <adutils/acquiredconf.hpp>
#include <adutils/acquireddata.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <regex>

using namespace adcontroller;

static size_t __nid__;

SampleProcessor::~SampleProcessor()
{
    fs_->close();
	Logging( L"Sample %1% closed in file '%2%'.", EventLog::pri_INFO ) % storage_name_.stem() % storage_name_.wstring();

    ADTRACE() << "SampleProcessor:: -- DTOR -- ";
}

SampleProcessor::SampleProcessor( boost::asio::io_service& io_service
                                  , std::shared_ptr< adcontrols::SampleRun > run ) : fs_( new adfs::filesystem )
                                                                                   , inProgress_( false )
                                                                                   , myId_( __nid__++ )
                                                                                   , strand_( io_service )
                                                                                   , objId_front_( 0 )
                                                                                   , pos_front_( 0 )
                                                                                   , stop_triggered_( false )
                                                                                   , sampleRun_( run )
{
}

void
SampleProcessor::prepare_storage( SignalObserver::Observer * masterObserver )
{
    // boost::filesystem::path path( adportable::profile::user_data_dir< char >() );
	// path /= "data";
	// path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
    boost::filesystem::path path( sampleRun_->dataDirectory() );

	if ( ! boost::filesystem::exists( path ) )
		boost::filesystem::create_directories( path );

    size_t runno = 0;
	if ( boost::filesystem::exists( path ) && boost::filesystem::is_directory( path ) ) {
        using boost::filesystem::directory_iterator;
		for ( directory_iterator it( path ); it != directory_iterator(); ++it ) {
            boost::filesystem::path fname = (*it);
			if ( fname.extension().string() == ".adfs" ) {
				std::string name = fname.stem().string();
				if ( ( name.size() == 8 ) 
                     && ( name[0] == 'R' || name[0] == 'r' )
                     && ( name[1] == 'U' || name[0] == 'u' )
                     && ( name[2] == 'N' || name[0] == 'n' )
                     && ( name[3] == '_' ) ) {
                    size_t no = atoi( name.substr( 4 ).c_str() );
                    if ( no > runno )
                        runno = no;
                }
            }
        }
	}

	std::ostringstream o;
	o << "RUN_" << std::setw(4) << std::setfill( '0' ) << runno + 1;
	boost::filesystem::path filename = path / o.str();
	filename.replace_extension( ".adfs" );

	storage_name_ = filename.normalize();
	
	///////////// creating filesystem ///////////////////
	if ( ! fs_->create( storage_name_.wstring().c_str() ) )
		return;

    adutils::AcquiredConf::create_table( fs_->db() );
    adutils::AcquiredData::create_table( fs_->db() );
    adutils::mscalibio::create_table( fs_->db() );
	
	populate_descriptions( masterObserver );
    populate_calibration( masterObserver );

	Logging( L"Sample processor '%1%' is ready to run.", EventLog::pri_INFO ) % storage_name_.wstring();
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

void
SampleProcessor::handle_data( unsigned long objId, long pos
                              , const SignalObserver::DataReadBuffer& rdBuf )
{
    if ( rdBuf.events & SignalObserver::wkEvent_INJECT ) {
        tp_inject_trigger_ = std::chrono::steady_clock::now(); // CAUTION: this has some unknown delay from exact trigger.
        if ( ! inProgress_ )
            Logging( L"Sample '%1%' got an INJECTION", EventLog::pri_INFO ) % storage_name_.wstring();
		inProgress_ = true;
    }

	if ( ! inProgress_ ) 
		return;

	adfs::stmt sql( fs_->db() );
	sql.prepare( "INSERT INTO AcquiredData VALUES( :oid, :time, :npos, :fcn, :events, :data, :meta )" );
	sql.begin();
	sql.bind( 1 ) = objId;
	sql.bind( 2 ) = rdBuf.uptime;
	sql.bind( 3 ) = pos;
	sql.bind( 4 ) = rdBuf.fcn;
	sql.bind( 5 ) = rdBuf.events;
	sql.bind( 6 ) = adfs::blob( rdBuf.xdata.length(), reinterpret_cast<const int8_t *>( rdBuf.xdata.get_buffer() ) );
	sql.bind( 7 ) = adfs::blob( rdBuf.xmeta.length(), reinterpret_cast<const int8_t *>( rdBuf.xmeta.get_buffer() ) ); // method
	if ( sql.step() == adfs::sqlite_done )
		sql.commit();
	else
		sql.reset();

    if ( objId == objId_front_ && pos_front_ > unsigned( pos + 10 ) ) {
        size_t nBehind = pos_front_ - pos;
        if ( stop_triggered_ && ( nBehind % 5 == 0 ) )
            Logging( L"'%1%' is being closed.  Saving %2% more data in background.", EventLog::pri_INFO ) % storage_name_.stem() % nBehind;
        else if ( nBehind % 25 == 0 )
            Logging( L"Sample '%1%' %2% data behind.", EventLog::pri_INFO ) % storage_name_.stem() % nBehind;
    }
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - tp_inject_trigger_);

    if ( elapsed_time.count() >= adcontrols::metric::scale_to_micro( sampleRun_->methodTime() ) ) {
        iTask::instance()->handle_stop_run();
    }

}

void
SampleProcessor::populate_calibration( SignalObserver::Observer * parent )
{
    SignalObserver::Observers_var vec = parent->getSiblings();

    if ( ( vec.ptr() != 0 ) && ( vec->length() > 0 ) ) {
        
        for ( CORBA::ULong i = 0; i < vec->length(); ++i ) {
            
            SignalObserver::Observer_ptr observer = vec[ i ];
            unsigned long objId = observer->objId();
            CORBA::WString_var dataClass;
            SignalObserver::octet_array_var data;
            CORBA::ULong idx = 0;
            while ( observer->readCalibration( idx++, data, dataClass ) ) {
                adfs::stmt sql( fs_->db() );
                sql.prepare( "INSERT INTO Calibration VALUES(:objid,:dataClass,:data,0)" );
                sql.bind( 1 ) = objId;
                sql.bind( 2 ) = std::wstring( dataClass.in() );
                sql.bind( 3 ) = adfs::blob( data->length(), reinterpret_cast< const int8_t *>( data->get_buffer() ) );
                if ( sql.step() == adfs::sqlite_done )
                    sql.commit();
                else
                    sql.reset();
            }
        }
        for ( CORBA::ULong i = 0; i < vec->length(); ++i )
            populate_calibration( vec[ i ] );
    }
    
}

void
SampleProcessor::populate_descriptions( SignalObserver::Observer * parent )
{
    SignalObserver::Observers_var vec = parent->getSiblings();

    unsigned long pobjId = parent->objId();

    if ( ( vec.ptr() != 0 ) && ( vec->length() > 0 ) ) {
        
        for ( CORBA::ULong i = 0; i < vec->length(); ++i ) {
            
            SignalObserver::Observer_ptr observer = vec[ i ];
            
            unsigned long objId = observer->objId();
            
			CORBA::WString_var clsid = observer->dataInterpreterClsid();
			SignalObserver::Description_var desc = observer->getDescription();
			CORBA::WString_var trace_id = desc->trace_id.in();
			CORBA::WString_var trace_display_name = desc->trace_display_name.in();
			CORBA::WString_var axis_x_label = desc->axis_x_label.in();
			CORBA::WString_var axis_y_label = desc->axis_y_label.in();

            adutils::AcquiredConf::insert( fs_->db()
                                           , objId
                                           , pobjId
                                           , std::wstring( clsid.in() )
                                           , uint64_t( desc->trace_method )
                                           , uint64_t( desc->spectrometer )
                                           , std::wstring( trace_id.in() )
                                           , std::wstring( trace_display_name.in() )
                                           , std::wstring( axis_x_label.in() )
                                           , std::wstring( axis_y_label.in() )
                                           , uint64_t( desc->axis_x_decimals )
                                           , uint64_t( desc->axis_y_decimals ) );
        }

        for ( CORBA::ULong i = 0; i < vec->length(); ++i )
            populate_descriptions( vec[ i ] );

    }
}
