// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "export_to_adfs.hpp"
#include "datafile.hpp"
#include "adutils/acquireddata.hpp"
#include "adutils/acquireddata_v3.hpp"
#include "datafile_factory.hpp"
#include "datareader.hpp"
#include "data_reader_ex.hpp"
#include <lrpfile.hpp>
#include <lrpheader.hpp>
#include <lrphead2.hpp>
#include <lrphead3.hpp>
#include <instsetup.hpp>
#include <lrpcalib.hpp>
#include <simions.hpp>
#include <lrptic.hpp>
#include <msdata.hpp>
#include "helper.hpp"
#include <adacquire/constants.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adportable/debug.hpp>
#include <adportable/bzip2.hpp>
#include <adutils/datafile_signature.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/format.hpp>
#include <chrono>

namespace shrader {

    class export_to_adfs::impl {
    public:
        std::shared_ptr< adfs::sqlite > db_;

        void create_acquired_conf() const {
            adfs::stmt sql( *db_ );
            sql.exec( "DROP TABLE IF EXISTS AcquiredConf" );
            sql.exec(
                "CREATE TABLE AcquiredConf ("
                " objuuid              UUID"
                ",objtext              TEXT"
                ",pobjuuid             UUID"
                ",dataInterpreterClsid TEXT"
                ",trace_method         INTEGER"
                ",spectrometer         INTEGER"
                ",trace_id             TEXT"
                ",trace_display_name   TEXT"
                ",axis_x_label         TEXT"
                ",axis_y_label         TEXT"
                ",axis_x_decimals      INTEGER"
                ",axis_y_decimals      INTEGER"
                ",UNIQUE (objuuid,trace_id)"
                ")"
                );
        }

        void create_lrptic_table() const {
            adfs::stmt sql( *db_ );
            sql.exec( "DROP TABLE IF EXISTS lrptic" );
            sql.exec(
                "CREATE TABLE lrptic ("
                " ptr        INTEGER PRIMARY KEY"
                ", time      INTEGER"
                ", intensity INTEGER"
                ", overload  INTEGER"
                ")"
                );
        }

        void create_msdata_table() const {
            adfs::stmt sql( *db_ );
            sql.exec( "DROP TABLE IF EXISTS msdata" );
            sql.exec(
                "CREATE TABLE msdata ("
                " id INTEGER PRIMARY KEY AUTOINCREMENT"
                ",scan INTEGER"
                ",block BLOB"
                ")"
                );
        }
    };
}

using namespace shrader;

export_to_adfs::~export_to_adfs()
{
}

export_to_adfs::export_to_adfs()
    : impl_( std::make_unique< impl >() )
{
}

export_to_adfs::export_to_adfs( std::shared_ptr< adfs::sqlite >&& db )
    : impl_( std::make_unique< impl >() )
{
    impl_->db_ = std::move( db );
}

bool
export_to_adfs::connect( std::shared_ptr< adfs::sqlite > db )
{
    impl_->db_ = db;
    return impl_->db_.get();
}

bool
export_to_adfs::operator()( const datafile& _ )
{
    adutils::data_signature::datafileSignature::create_table( *impl_->db_ );
    adutils::v3::AcquiredData::create_table_v3( *impl_->db_ );

    adfs::stmt sql( *impl_->db_ );
    sql.exec( "DELETE FROM AcquiredData" );

    using namespace adutils::data_signature;
    sql << datum_t{ "creator", value_t( lrp_ex::data_reader::__objuuid__() ) }; //impl_->uuid_ ) };
    sql << datum_t{ "create_date", value_t( std::chrono::system_clock::now() ) };
    sql << datum_t{ "datafile_factory", value_t( std::string(datafile_factory::instance()->iid() ) ) };

    // std::unique_ptr< shrader::lrpheader > header_;  // 256 o
    // std::unique_ptr< shrader::lrphead2 > header2_;  // 256 o
    // std::unique_ptr< shrader::lrphead3 > header3_;  // 256 o
    // std::unique_ptr< shrader::instsetup > instsetup_; // 256 o
    // std::unique_ptr< shrader::lrpcalib > lrpcalib_;  // 256 o
    // std::unique_ptr< shrader::simions > simions_;    // 256 o
    if ( auto lrp = _.lrpfile() ) {

        if ( auto d = lrp->header() )
            sql << datum_t{ "lrpheader", value_t( block_to_string( d.rp() ) ) }; // bzip2, base64

        if ( auto d = lrp->header2() )
            sql << datum_t{ "lrpheader2", value_t( block_to_string( d.rp() ) ) };  // bzip2, base64

        if ( auto d = lrp->header3() )
            sql << datum_t{ "lrpheader3", value_t( block_to_string( d.rp() ) ) };  // bzip2, base64

        if ( auto d = lrp->instsetup() )
            sql << datum_t{ "instsetup", value_t( block_to_string( d.rp() ) ) };  // bzip2, base64

        if ( auto d = lrp->lrpcalib() )
            sql << datum_t{ "calib", value_t( block_to_string( d.rp() ) ) };  // bzip2, base64

        if ( auto d = lrp->simions() )
            sql << datum_t{ "calib", value_t( block_to_string( d.rp() ) ) };  // bzip2, base64

    }

    do {
        impl_->create_acquired_conf();
        bool success = sql.prepare(
            "INSERT OR REPLACE INTO \
 AcquiredConf VALUES(    \
:objuuid                 \
,:objtext                \
,:pobjuuid               \
,:dataInterpreterClsid   \
,:trace_method           \
,:spectrometer           \
,:trace_id               \
,:trace_display_name     \
,:axis_x_label           \
,:axis_y_label           \
,:axis_x_decimails       \
,:axis_y_decimals        \
)" );
        for ( const auto& reader: _.dataReaders() ) {
            int col = 1;
            // ADDEBUG() << "\t traceid: " << reader.second->traceid();
            sql.bind( col++ ) = lrp_ex::data_reader::__objuuid__(); // objid; <-- 6a6df573-...
            sql.bind( col++ ) = lrp_ex::data_reader::__objtext__(); // d.objtext; <-- 1.admzml.ms-cheminfo.com
            sql.bind( col++ ) = boost::uuids::uuid{0}; // d.pobjid;
            sql.bind( col++ ) = std::string( "TSSlrp" ); // dataInterpreterClsid
            sql.bind( col++ ) = int64_t( adacquire::SignalObserver::eTRACE_SPECTRA ); // trace_method
            sql.bind( col++ ) = int64_t( adacquire::SignalObserver::eMassSpectrometer ); // spectrometer
            sql.bind( col++ ) = reader->trace_id();
            sql.bind( col++ ) = std::string{};            // trace_display_name/
            sql.bind( col++ ) = std::string("Time");      // axis_x_label
            sql.bind( col++ ) = std::string("Count");     // axis_y_label
            sql.bind( col++ ) = 2;                        // axis_x_decimals
            sql.bind( col++ ) = 0;                        // axis_y_decimals
            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << sql.errmsg();
        };
    } while ( 0 );

    // TIC
    if ( auto lrp = _.lrpfile() ) {
        impl_->create_lrptic_table();
        bool success = sql.prepare(
            "INSERT OR REPLACE INTO lrptic VALUES( :ptr,:time,:tic,:overload )" );

        for ( const auto& tic: lrp->lrptic().tic() ) {
            int col = 1;
            sql.bind( col++ ) = tic.ptr;
            sql.bind( col++ ) = tic.time;
            sql.bind( col++ ) = tic.intensity;
            sql.bind( col++ ) = tic.overload;
            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << sql.errmsg();
        }
    }

    // msdata
    if ( auto lrp = _.lrpfile() ) {
        impl_->create_msdata_table();
        bool success = sql.prepare(
            "INSERT OR REPLACE INTO msdata VALUES(:scan,:block )" );
        for ( const auto& msdata: lrp->msdata() ) {
            const char * rp = reinterpret_cast< const char * >(msdata->blocks().data());
            auto blob = bzip2_compress( std::string( rp, msdata->blocks().size() * sizeof( detail::block ) ) );
            int col = 1;
            sql.bind( col++ ) = int( msdata->blocks().at(0).scan);
            sql.bind( col++ ) = adfs::blob( blob.size(), blob.data() );
            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << sql.errmsg();
        }
    }

    return true;
}
