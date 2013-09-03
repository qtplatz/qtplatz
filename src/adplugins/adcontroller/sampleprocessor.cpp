/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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
#include <adinterface/signalobserverC.h>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/folder.hpp>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <regex>

using namespace adcontroller;

SampleProcessor::~SampleProcessor()
{
}

SampleProcessor::SampleProcessor() : fs_( new adfs::filesystem )
								   , inProgress_( false )
{
}

void
SampleProcessor::prepare_storage( SignalObserver::Observer * masterObserver )
{
    boost::filesystem::path path( adportable::profile::user_data_dir< char >() );
	path /= "data";
	path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );

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

	create_acquiredconf_table();
	create_acquireddata_table();
	
	populate_descriptions( masterObserver );
}

void
SampleProcessor::handle_data( unsigned long objId, long pos
                              , const SignalObserver::DataReadBuffer& rdBuf )
{
    if ( rdBuf.events & SignalObserver::wkEvent_INJECT ) {
        adportable::debug(__FILE__, __LINE__) << "Got INJECT at " << pos;
		inProgress_ = true;
	}
	if ( ! inProgress_ ) 
		return;
	adfs::stmt sql( fs_->db() );
	sql.prepare( "INSERT INTO AcquiredData VALUES( :oid, :time, :npos, :ndata, :events, :data, :meta )" );
	sql.begin();
	sql.bind( 1 ) = objId;
	sql.bind( 2 ) = rdBuf.uptime;
	sql.bind( 3 ) = pos;
	sql.bind( 4 ) = rdBuf.ndata;
	sql.bind( 5 ) = rdBuf.events;
	sql.bind( 6 ) = adfs::blob( rdBuf.xdata.length(), reinterpret_cast<const int8_t *>( rdBuf.xdata.get_buffer() ) );
	sql.bind( 7 ) = adfs::blob( rdBuf.xmeta.length(), reinterpret_cast<const int8_t *>( rdBuf.xmeta.get_buffer() ) ); // method
	if ( sql.step() == adfs::sqlite_done )
		sql.commit();
	else
		sql.reset();
}

void
SampleProcessor::create_acquireddata_table()
{
	adfs::stmt sql( fs_->db() );
	sql.exec( 
        "CREATE TABLE AcquiredData \
(oid    INTEGER                    \
,time   INTEGER                    \
,npos   INTEGER                    \
,ndata  INTEGER                    \
,events INTEGER                    \
,data   BLOB                       \
,meta   BLOB                       \
)"
        );
}

void
SampleProcessor::create_acquiredconf_table()
{
	adfs::stmt sql( fs_->db() );
    sql.exec(
        "CREATE TABLE AcquiredConf (\
 objid                INTEGER       \
,pobjid               INTEGER       \
,dataInterpreterClsid TEXT          \
,trace_method         INTEGER       \
,spectrometer         INTEGER       \
,trace_id             TEXT          \
,trace_display_name   TEXT          \
,axis_x_label         TEXT          \
,axis_y_label         TEXT          \
,axis_x_decimals      INTEGER       \
,axis_y_decimals      INTEGER       \
,UNIQUE(objid)                      \
)" 
        );
}

void
SampleProcessor::populate_descriptions( SignalObserver::Observer * parent )
{
    SignalObserver::Observers_var vec = parent->getSiblings();

    unsigned long pobjId = parent->objId();

    if ( ( vec.ptr() != 0 ) && ( vec->length() > 0 ) ) {

        for ( size_t i = 0; i < vec->length(); ++i ) {
            
            SignalObserver::Observer_ptr observer = vec[ i ];
            
            unsigned long objId = observer->objId();
            
			CORBA::WString_var clsid = observer->dataInterpreterClsid();
			SignalObserver::Description_var desc = observer->getDescription();
			CORBA::WString_var trace_id = desc->trace_id.in();
			CORBA::WString_var trace_display_name = desc->trace_display_name.in();
			CORBA::WString_var axis_x_label = desc->axis_x_label.in();
			CORBA::WString_var axis_y_label = desc->axis_y_label.in();
            
            adfs::stmt sql( fs_->db() );
            sql.prepare( 
                "INSERT INTO AcquiredConf VALUES(\
:objid                                           \
,:pobjid                                         \
,:dataInterpreterClsid                           \
,:trace_method                                   \
,:spectrometer                                   \
,:trace_id                                       \
,:trace_display_name                             \
,:axis_x_label                                   \
,:axis_y_label                                   \
,:axis_x_decimails                               \
,:axis_y_decimals                                \
)" );

            sql.begin();
            sql.bind( 1 ) = objId;
            sql.bind( 2 ) = pobjId;
            sql.bind( 3 ) = std::wstring( clsid.in() );
            sql.bind( 4 ) = static_cast< unsigned long >( desc->trace_method );
            sql.bind( 5 ) = static_cast< unsigned long >( desc->spectrometer );
            sql.bind( 6 ) = std::wstring( trace_id.in() );
            sql.bind( 7 ) = std::wstring( trace_display_name.in() );
            sql.bind( 8 ) = std::wstring( axis_x_label.in() );
            sql.bind( 9 ) = std::wstring( axis_y_label.in() );
            sql.bind( 10 ) = static_cast< unsigned long >( desc->axis_x_decimals );
            sql.bind( 11 ) = static_cast< unsigned long >( desc->axis_y_decimals );
			if ( sql.step() == adfs::sqlite_done )
				sql.commit();
			else
				sql.reset();
        }

        for ( size_t i = 0; i < vec->length(); ++i )
            populate_descriptions( vec[ i ] );

    }
}
