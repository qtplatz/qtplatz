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
#include "accession.hpp"
#include "adutils/acquireddata.hpp"
#include "adutils/acquireddata_v3.hpp"
#include "datafile_factory.hpp"
#include "datareader.hpp"
#include "datareader_ex.hpp"
#include "mzml.hpp"
#include "mzmldatumbase.hpp"
#include "scan_protocol.hpp"
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

namespace mzml {
    class export_to_adfs::impl {
    public:
        std::shared_ptr< adfs::sqlite > db_;

        // 9C457C8F-D0B6-44EA-98F5-E89F8A229A63
        // static constexpr const boost::uuids::uuid uuid_ =
        // { 0x9C, 0x45, 0x7C, 0x8F, 0xD0, 0xB6, 0x44, 0xEA
        //   , 0x98, 0xF5, 0xE8, 0x9F, 0x8A, 0x22, 0x9A, 0x63 };

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

        //
        void create_chromatogram_table() const {
            adfs::stmt sql( *db_ );
            sql.exec( "DROP TABLE IF EXISTS Chromatograms" );
            sql.exec(
                "CREATE TABLE Chromatograms ("
                " objuuid              UUID"    // 0 this->uuid_ (reserved for future use)
                ",fcn                  INTEGER" // 1
                ",mode                 TEXT"    // 2 SIM,SRM,TIC,BPI...
                ",ms_level             INTEGER" // 3
                ",ion_polarity         TEXT"    // 4
                ",precursor_mz         REAL"    // 5
                ",collision_energy     REAL"    // 6
                ",meta                 BLOB"    // 7
                ",dataUUID             UUID"    // 8 Chromatogram::__clsid__
                ",dataClass            TEXT"    // 9 Chromatogram::dataClass
                ",data                 BLOB"    // 10
                ")"
                );
        }

    };
}

namespace {

    /////////////////////////
    std::string monitor_mode( int ms_level ) {
        switch( ms_level ) {
        case 1: return "SIM";
        case 2: return "SRM";
        }
        return (boost::format( "MS^%d" ) % ms_level).str();
    }

    std::string polarity_text( mzml::ion_polarity_type t )
    {
        switch( t ) {
        case mzml::polarity_positive: return "positive_polarity";
        case mzml::polarity_negative: return "negative_polarity";
        case mzml::polarity_unknown: return "unknown_polarity";
        }
    }

}

using namespace mzml;

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
export_to_adfs::operator()( const mzML& _ )
{
    adutils::data_signature::datafileSignature::create_table( *impl_->db_ );
    adutils::v3::AcquiredData::create_table_v3( *impl_->db_ );
    impl_->create_chromatogram_table();

    adfs::stmt sql( *impl_->db_ );
    sql.exec( "DELETE FROM AcquiredData" );

    using namespace adutils::data_signature;
    sql << datum_t{ "creator", value_t( exposed::data_reader::__objuuid__() ) }; //impl_->uuid_ ) };
    sql << datum_t{ "create_date", value_t( std::chrono::system_clock::now() ) };
    sql << datum_t{ "datafile_factory", value_t( std::string(datafile_factory::instance()->iid() ) ) };

    if ( auto d = _.get_fileDescription() ) {
        sql << datum_t{ "fileDescription", value_t( boost::json::value_from( *d ) ) };
    }
    if ( auto d = _.get_softwareList() ) {
        sql << datum_t{ "softwareList", value_t( boost::json::value_from( *d ) ) };
    }
    if ( auto d = _.get_instrumentConfigurationList() ) {
        sql << datum_t{ "instrumentConfigurationList", value_t( boost::json::value_from( *d ) ) };
    }
    if ( auto d = _.get_dataProcessingList() ) {
        sql << datum_t{ "dataProcessingList", value_t( boost::json::value_from( *d ) ) };
    }

    if ( auto run = _.xml_document().select_node( "//run" ) ) {
        sql << datum_t{ "run", boost::json::value{
                { "defaultInstrumentConfigurationRef", run.node().attribute( "defaultInstrumentConfigurationRef" ).value() }
                    , { "defaultSourceFileRef",  run.node().attribute( "defaultSourceFileRef" ).value() }
                    , { "defaultSourceFileRef",  run.node().attribute( "defaultSourceFileRef" ).value() }
                    , { "defaultSourceFileRef",  run.node().attribute( "defaultSourceFileRef" ).value() }
                    , { "id",  run.node().attribute( "id" ).value() }
            }};
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
        for ( const auto& reader: _.dataReaderMap() ) {
            int col = 1;
            // ADDEBUG() << "\t traceid: " << reader.second->traceid();
            sql.bind( col++ ) = exposed::data_reader::__objuuid__(); // objid; <-- 6a6df573-...
            sql.bind( col++ ) = exposed::data_reader::__objtext__(); // d.objtext; <-- 1.admzml.ms-cheminfo.com
            sql.bind( col++ ) = boost::uuids::uuid{0}; // d.pobjid;
            sql.bind( col++ ) = std::string( "mzML" ); // dataInterpreterClsid
            sql.bind( col++ ) = int64_t( adacquire::SignalObserver::eTRACE_SPECTRA );
            sql.bind( col++ ) = int64_t( adacquire::SignalObserver::eMassSpectrometer );
            sql.bind( col++ ) = reader.second->traceid(); //
            sql.bind( col++ ) = std::string{};
            sql.bind( col++ ) = std::string("Time");
            sql.bind( col++ ) = std::string("Count");
            sql.bind( col++ ) = 2;
            sql.bind( col++ ) = 0;
            if ( sql.step() != adfs::sqlite_done )
                ADDEBUG() << sql.errmsg();
        };
    } while ( 0 );

    size_t wcounts(0);
    for ( const auto [scan_id,sp]: _.scan_indices() ) {
        // scan_id = std::tuple< int, string, double (time), scan_protocol
        // sp = mzMLSpectrum
        auto xml = sp->serialize();
        std::string xdata;
        adportable::bzip2::compress( xdata, xml.data(), xml.size() );

        { // verify
            std::string inflated;
            adportable::bzip2::decompress( inflated, xdata.data(), xdata.size() );
            auto spc = serializer::deserialize( inflated.data(), inflated.size() );
            // ADDEBUG() << scan_id
            //           << "\tcomp.ratio=" << double(xml.size()) / xml.size();
        }
        scan_id_accessor scan_ident( scan_id );
        const uint32_t events = ( wcounts++ == 0 ) ?
            adacquire::SignalObserver::wkEvent::wkEvent_AcqInProgress | adacquire::SignalObserver::wkEvent::wkEvent_INJECT
            : adacquire::SignalObserver::wkEvent::wkEvent_AcqInProgress;

        std::string xmeta = boost::json::serialize( boost::json::value_from( scan_id ) );

        if ( ! adutils::v3::AcquiredData::insert(
                 *impl_->db_
                 , exposed::data_reader::__objuuid__() // objid; <-- 6a6df573-...
                 , uint64_t( scan_ident.scan_start_time() * 1e9 ) // elapsed_time
                 , uint64_t( scan_ident.scan_start_time() * 1e9 ) // epoch_time
                 , scan_ident.scan_index() // pos
                 , sp->protocol_id()   // fcn
                 , sp->length()    // ndata (number of data in the buffer
                 , events
                 , xdata  // xdata
                 , xmeta  // xmeta
                 ) ) {
        }
    }

    sql.prepare( "INSERT INTO Chromatograms "
                 "(objuuid,fcn,mode,ms_level,ion_polarity,precursor_mz,collision_energy,meta,dataUUID,dataClass,data)"
                 "VALUES(?,?,?,?,?,?,?,?,?,?,?)" );
    for ( const auto& [fcn, chro, node]: _.SRMs() ) {
        auto scan_protocol = std::get< enum_scan_protocol >(mzml::scan_identifier{}( node ));
        sql.bind(1) = exposed::data_reader::__objuuid__(); //impl_->uuid_;
        sql.bind(2) = adfs::null{}; // fcn;
        sql.bind(3) = monitor_mode( scan_protocol.ms_level() );
        sql.bind(4) = scan_protocol.ms_level();
        sql.bind(5) = polarity_text( scan_protocol.polarity() );
        if ( scan_protocol.ms_level() >= 2 ) {
            sql.bind(6) = scan_protocol.precursor_mz();
            sql.bind(7) = scan_protocol.collision_energy();
        } else {
            sql.bind(6) = adfs::null{};
            sql.bind(7) = adfs::null{};
        }
        auto xmeta = archive_to_string( node, true );
        auto xdata = archive_to_string( *chro, true );
        sql.bind(8) = adfs::blob( xmeta.size(), xmeta.data() ); // blob_helper( archive_to_string( node, true ) ).blob();
        sql.bind(9) = adcontrols::Chromatogram::__clsid__();
        sql.bind(10) = std::wstring(adcontrols::Chromatogram::dataClass());
        sql.bind(11) = adfs::blob( xdata.size(), xdata.data() ); // blob_helper( archive_to_string( *chro, true )).blob();
        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "Error: " << sql.errmsg();
        sql.reset();
    }

    sql.prepare( "INSERT INTO Chromatograms "
                 "(objuuid,fcn,mode,dataUUID,dataClass,data)"
                 "VALUES(?,?,?,?,?,?)" );
    for ( const auto& [fcn, chro]: _.TICs() ) {
        auto zc = archive_to_string( *chro, true );
        sql.bind(1) = exposed::data_reader::__objuuid__();// impl_->uuid_;
        sql.bind(2) = fcn;
        sql.bind(3) = std::string("TIC");
        sql.bind(4) = adcontrols::Chromatogram::__clsid__();
        sql.bind(5) = std::wstring(adcontrols::Chromatogram::dataClass());
        sql.bind(6) = adfs::blob( zc.size(), zc.data() );
        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "Error: " << sql.errmsg();
        sql.reset();
    }

    sql.prepare( "INSERT INTO Chromatograms "
                 "(objuuid,mode,dataClass,data) "
                 "VALUES(?,?,?,?)" );
    for ( auto chro: _.mzMLChromatograms() ) {
        sql.bind(1) = exposed::data_reader::__objuuid__(); // impl_->uuid_;
        sql.bind(2) = chro->id();
        sql.bind(3) = std::string("XML");
        auto xdata = archive_to_string( chro->node(), true );
        sql.bind(4) = adfs::blob( xdata.size(), xdata.data() ); // blob_helper( archive_to_string( chro->node(), true ) ).blob();
        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "Error: " << sql.errmsg();
        sql.reset();
        auto zml = archive_to_string( chro->node(), true );
        ADDEBUG() << "is_compresssed? " << adportable::bzip2::is_a( zml.data(), zml.size() );
    }
    return true;
}
