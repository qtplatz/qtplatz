/**************************************************************************
 ** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
 ** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
 *
 ** Contact: toshi.hondo@qtplatz.com
 **
 ** Commercial Usage
 **
 ** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
 ** accordance with the MS-Cheminformatics Commercial License Agreement provided with
 ** the Software or, alternatively, in accordance with the terms contained in
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

#include "datareader.hpp"
#include "datainterpreter.hpp"
#include <socfpga/constants.hpp>
#include <socfpga/advalue.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adcontrols/waveform.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/utf.hpp>
#include <adutils/acquiredconf_v3.hpp>
#include <boost/format.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <atomic>
#include <cassert>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#if defined _MSC_VER && _MSC_VER <= 1800
# include <compiler/make_unique.hpp>
#endif

namespace socfpgainterpreter {

    class DataReader::impl {
    public:
        std::vector< socfpga::dgmod::advalue > data_;
        socfpga::dgmod::advalue injdata_;
        std::once_flag flag_;
    };
}

using namespace socfpgainterpreter;

DataReader::~DataReader()
{
}

DataReader::DataReader( const char * traceid ) : adcontrols::DataReader( traceid )
                                               , objid_( {{ 0 }} )
                                               , objrowid_( -1 )
                                               , fcnCount_( 0 )
                                               , elapsed_time_origin_( 0 )
                                               , impl_( new impl() )
{
    interpreter_ = std::make_unique< socfpgainterpreter::DataInterpreter >();
    display_name_ = "1.ADC";
}

// static
std::vector< std::string >
DataReader::traceid_list()
{
    static const std::vector< std::string > list = { socfpga::dgmod::trace_observer_name };
    return list;
}

bool
DataReader::initialize( adfs::filesystem& dbf, const boost::uuids::uuid& objid, const std::string& objtext )
{
    if ( interpreter_ ) {

        objid_ = objid; // objid tells channel/module id
        objtext_ = objtext; // for debugging convension
        db_ = dbf._ptr();

        if ( auto db = db_.lock() ) {
            {
                adfs::stmt sql( *db );
                sql.prepare( "SELECT rowid FROM AcquiredConf WHERE objuuid = ?" );
                sql.bind( 1 ) = objid_;
                if ( sql.step() == adfs::sqlite_row )
                    objrowid_ = sql.get_column_value< int64_t >( 0 );
            }
            // fcnCount
            {
                adfs::stmt sql( *db );
                sql.prepare( "SELECT COUNT( DISTINCT fcn ) FROM AcquiredData WHERE objuuid = ?" );
                sql.bind( 1 ) = objid_;
                if ( sql.step() == adfs::sqlite_row )
                    fcnCount_ = sql.get_column_value< int64_t >( 0 );
            }

#if ! defined NDEBUG
            ADDEBUG() << "DataReader::initailze(" << objid << ", " << objtext << ") fcnCount=" << fcnCount_;
#endif
        }
        return true;
    }
    return false;
}

void
DataReader::finalize()
{
}


const boost::uuids::uuid&
DataReader::objuuid() const
{
    return objid_;
}

const std::string&
DataReader::objtext() const
{
    return objtext_;
}

int64_t
DataReader::objrowid() const
{
    return objrowid_;
}

const std::string&
DataReader::display_name() const
{
    return display_name_;
}

size_t
DataReader::fcnCount() const
{
    // skip timecount data -- too large to handle in the dataproc
    return fcnCount_;
}

size_t
DataReader::size( int fcn ) const
{
    if ( auto db = db_.lock() ) {
        adfs::stmt sql( *db );
        sql.prepare( "SELECT COUNT(*) FROM AcquiredData WHERE objuuid=?" );
        sql.bind( 1 ) = objid_;
        if ( sql.step() == adfs::sqlite_row )
            return sql.get_column_value< int64_t >( 0 );
    }
    return 0;
}

adcontrols::DataReader::const_iterator
DataReader::begin( int fcn ) const
{
    if ( auto db = db_.lock() ) {
        adfs::stmt sql( *db );
        if ( fcn >= 0 ) {
            sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid=? AND fcn=? ORDER BY npos LIMIT 1" );
            sql.bind( 1 ) = objid_;
            sql.bind( 2 ) = fcn;
        } else {
            // choose min(rowid) due to protocol w/ complex replicates combination
            sql.prepare( "SELECT rowid FROM AcquiredData WHERE objuuid=? ORDER BY rowid LIMIT 1" );
            sql.bind( 1 ) = objid_;
        }
        if ( sql.step() == adfs::sqlite_row ) {
            auto rowid = sql.get_column_value< int64_t >( 0 );
            return adcontrols::DataReader_iterator( this, rowid, fcn );
        }
    }
    return end();
}

adcontrols::DataReader::const_iterator
DataReader::end() const
{
    return adcontrols::DataReader_iterator( this, (-1) );
}

adcontrols::DataReader::const_iterator
DataReader::findPos( double seconds, int fcn, bool closest, TimeSpec tspec ) const
{
    ADDEBUG() << "findPos( " << int64_t( seconds * std::nano::den ) << ")";

    if ( tspec == ElapsedTime ) {
        if ( auto db = db_.lock() ) {
            adfs::stmt sql( *db );
            if ( closest ) {
                sql.prepare(
                    "SELECT rowid FROM AcquiredData WHERE "
                    "objuuid=:objid "
                    "AND fcn=? "
                    "ORDER BY ABS( elapsed_time - (SELECT MIN(elapsed_time) + ? FROM AcquiredData)) LIMIT 1");
            } else {
                sql.prepare(
                    "SELECT rowid FROM AcquiredData WHERE "
                    "objuuid=:objid "
                    "AND fcn=? "
                    "AND elapsed_time >= (SELECT MIN(elapsed_time) + ? FROM AcquiredData) LIMIT 1");
            }
            sql.bind( 1 ) = objid_;
            sql.bind( 2 ) = ( fcn < 0 ? 0 : fcn );
            sql.bind( 3 ) = uint64_t( seconds * std::nano::den );

            if ( sql.step() == adfs::sqlite_row ) {
                auto rowid = sql.get_column_value< int64_t >( 0 );
                return adcontrols::DataReader_iterator( this, rowid, fcn );
            }
        }
    }

    return end();
}

double
DataReader::findTime( int64_t pos, IndexSpec ispec, bool exactMatch ) const
{
    assert( ispec == TriggerNumber );
    ADDEBUG() << "findTime( " << pos << ")";

    if ( auto db = db_.lock() ) {
        adfs::stmt sql( *db );
        if ( exactMatch ) {
            sql.prepare("SELECT elapsed_time - (SELECT MIN(elapsed_time) FROM AcquiredData) "
                        "FROM AcquiredData WHERE objuuid=? AND fcn=? AND npos = ? LIMIT 1" );
        } else {
            sql.prepare("SELECT elapsed_time - (SELECT MIN(elapsed_time) FROM AcquiredData) "
                        "FROM AcquiredData WHERE objuuid=? AND fcn=? AND npos >= ? LIMIT 1" );
        }
        if ( sql.step() == adfs::sqlite_row ) {
            auto nanoseconds = sql.get_column_value< int64_t >( 0 );
            return double( nanoseconds ) / std::nano::den;
        }
    }
    return -1.0;
}

std::shared_ptr< const adcontrols::Chromatogram >
DataReader::TIC( int fcn ) const
{
    ADDEBUG() << __FUNCTION__ << "(" << fcn << ")";

    std::call_once( impl_->flag_, [&]{
            if ( auto interpreter = interpreter_->_narrow< socfpgainterpreter::DataInterpreter >() ) {
                if ( auto db = db_.lock() ) {
                    adfs::stmt sql( *db );
                    sql.prepare( "SELECT data FROM AcquiredData WHERE objuuid = ? ORDER BY npos" );
                    sql.bind( 1 ) = objid_;
                    while ( sql.step() == adfs::sqlite_row ) {
                        adfs::blob xdata = sql.get_column_value< adfs::blob >( 0 );
                        if ( interpreter->translate( impl_->data_, xdata.data(), xdata.size() ) == adcontrols::translate_complete )
                            ;
                    }
                    auto it = std::find_if( impl_->data_.begin(), impl_->data_.end(), [](const auto& a){ return a.flags & adacquire::SignalObserver::wkEvent_INJECT; } );
                    if ( it == impl_->data_.end() )
                        it = impl_->data_.begin();
                    impl_->injdata_ = *it;
                }
            }
        });

    auto ptr = std::make_shared< adcontrols::Chromatogram >();
    ptr->setDataReaderUuid( objid_ );

    const auto& injdata = impl_->injdata_;

    for ( const auto& item: impl_->data_ ) {
        double time = double( item.elapsed_time - injdata.flags_time ) / std::nano::den;
        double value = item.ad[ 0 ];
#ifndef NDEBUG
        ADDEBUG() << "data: " << time << "s, " << value << "mV";
#endif
        (*ptr) << std::make_pair( time, value );
    }

    return ptr;
}


int64_t
DataReader::next( int64_t rowid ) const
{
    ADDEBUG() << __FUNCTION__ << "(" << rowid << ")";
    return -1;
}

int64_t
DataReader::next( int64_t rowid, int fcn ) const
{
    ADDEBUG() << __FUNCTION__ << "(" << rowid << ")";
    return (-1);
}

int64_t
DataReader::pos( int64_t rowid ) const
{
    ADDEBUG() << __FUNCTION__ << "(" << rowid << ")";
    return -1;
}

int64_t
DataReader::elapsed_time( int64_t rowid ) const
{
    ADDEBUG() << __FUNCTION__ << "(" << rowid << ")";
    return -1;
}

double
DataReader::time_since_inject( int64_t rowid ) const
{
    ADDEBUG() << __FUNCTION__ << "(" << rowid << ")";
    return -1;
}

int
DataReader::fcn( int64_t rowid ) const
{
    ADDEBUG() << __FUNCTION__ << "(" << rowid << ")";
    return -1;
}

boost::any
DataReader::getData( int64_t rowid ) const
{
    ADDEBUG() << __FUNCTION__ << "(" << rowid << ")";
    return nullptr;
}

std::shared_ptr< adcontrols::Chromatogram >
DataReader::getChromatogram( int fcn, double time, double width ) const
{
    return nullptr;
}

adcontrols::DataInterpreter *
DataReader::dataInterpreter() const
{
    return interpreter_.get();
}
