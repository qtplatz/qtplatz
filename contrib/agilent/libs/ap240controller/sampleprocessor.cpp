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
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <adfs/sqlite.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adportable/bzip2.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
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

using namespace mpxcontroller;
namespace so = adicontroller::SignalObserver;

static size_t __nid__;

SampleProcessor::~SampleProcessor()
{
    close();
}

SampleProcessor::SampleProcessor( boost::asio::io_service& io_service
                                  , std::shared_ptr< adcontrols::SampleRun > run
                                  , std::shared_ptr< adcontrols::ControlMethod::Method > cmth )
    : fs_( new adfs::filesystem )
    , inProgress_( false )
    , myId_( __nid__++ )
    , strand_( io_service )
    , objId_front_( 0 )
    , pos_front_( 0 )
    , stop_triggered_( false )
    , sampleRun_( run )
    , ctrl_method_( cmth )
{
    tp_inject_trigger_ = std::chrono::steady_clock::now();
}

void
SampleProcessor::close()
{
    fs_->close();
    boost::filesystem::path running_name( storage_name_ );
    storage_name_.replace_extension( ".adfs" );
    boost::filesystem::rename( running_name, storage_name_ );
}

void
SampleProcessor::prepare_storage( std::shared_ptr< so::Observer > masterObserver )
{
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
                     && ( name[0] == 'M' || name[0] == 'm' )
                     && ( name[1] == 'P' || name[0] == 'p' )
                     && ( name[2] == 'X' || name[0] == 'x' )
                     && ( name[3] == '_' ) ) {
                    size_t no = atoi( name.substr( 4 ).c_str() );
                    if ( no > runno )
                        runno = no;
                }
            }
        }
	}

	std::ostringstream o;
	o << "MPX_" << std::setw(4) << std::setfill( '0' ) << runno + 1;
	boost::filesystem::path filename = path / o.str();
    filename.replace_extension( ".adfs-in-progress" );

	storage_name_ = filename.normalize();
	
	///////////// creating filesystem ///////////////////
	if ( ! fs_->create( storage_name_.wstring().c_str() ) )
		return;

    adutils::AcquiredConf::create_table( fs_->db() );
    adutils::AcquiredData::create_table( fs_->db() );
    adutils::mscalibio::create_table( fs_->db() );
	
    populate_descriptions( masterObserver.get() );
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
                              , const so::DataReadBuffer& rdBuf )
{
    if ( rdBuf.events() & so::wkEvent_INJECT ) {
        tp_inject_trigger_ = std::chrono::steady_clock::now(); // CAUTION: this has some unknown delay from exact trigger.
		inProgress_ = true;
    }

	if ( ! inProgress_ ) 
		return;

    std::string xdata, xmeta;
    
    adportable::bzip2::compress( xdata, reinterpret_cast<const char *>( rdBuf.xdata().data() ), rdBuf.xdata().size() );
    adportable::bzip2::compress( xmeta, reinterpret_cast<const char *>( rdBuf.xmeta().data() ), rdBuf.xmeta().size() );

	adfs::stmt sql( fs_->db() );
	sql.prepare( "INSERT INTO AcquiredData VALUES( :oid, :time, :npos, :fcn, :events, :data, :meta )" );
	sql.begin();
	sql.bind( 1 ) = objId;
    sql.bind( 2 ) = rdBuf.timepoint();
	sql.bind( 3 ) = pos;
	sql.bind( 4 ) = rdBuf.fcn();
	sql.bind( 5 ) = rdBuf.events();
    sql.bind( 6 ) = adfs::blob( xdata.size(), reinterpret_cast<const int8_t *>( xdata.data() ) );
    if ( xmeta.size() < rdBuf.xmeta().size() )
        sql.bind( 7 ) = adfs::blob( xmeta.size(), reinterpret_cast<const int8_t *>( xmeta.data() ) ); // method
    else
        sql.bind( 7 ) = adfs::blob( rdBuf.xmeta().size(), reinterpret_cast<const int8_t *>( rdBuf.xmeta().data() ) ); // method

	if ( sql.step() == adfs::sqlite_done )
		sql.commit();
	else
		sql.reset();

    if ( objId == objId_front_ && pos_front_ > unsigned( pos + 10 ) ) {
        size_t nBehind = pos_front_ - pos;
        if ( stop_triggered_ && ( nBehind % 5 == 0 ) )
            ADDEBUG() << boost::format( "'%1%' is being closed.  Saving %2% more data in background.") % storage_name_.stem() % nBehind;
    }

    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - tp_inject_trigger_);
    
    //ADDEBUG() << "handle_data  elapsed time: " << elapsed_time.count() << ", methodTime: " << sampleRun_->methodTime();

    if ( elapsed_time.count() >= sampleRun_->methodTime() ) {
        inProgress_ = false;
    }

}

void
SampleProcessor::populate_descriptions( so::Observer * parent )
{
    auto self( parent->shared_from_this() );

    for ( auto& sibling : parent->siblings() ) {

        adutils::AcquiredConf::insert( fs_->db()
                                       , sibling->objId()
                                       , parent->objId()
                                       , sibling->uuid()
                                       , sibling->dataInterpreterClsid()
                                       , uint32_t( sibling->description().trace_method() )
                                       , uint32_t( sibling->description().spectrometer() )
                                       , sibling->description().trace_id()
                                       , sibling->description().trace_display_name()
                                       , sibling->description().axis_label( so::Description::axisX )
                                       , sibling->description().axis_label( so::Description::axisY )
                                       , sibling->description().axis_decimals( so::Description::axisX )
                                       , sibling->description().axis_decimals( so::Description::axisY ) );

        populate_descriptions( sibling.get() );
    }
}

const boost::filesystem::path&
SampleProcessor::storage_name() const
{
    return storage_name_;
}
