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
#include "adportfolio/node.hpp"
#include "adutils/acquireddata.hpp"
#include "adutils/acquireddata_v3.hpp"
#include "datareader.hpp"
#include "datareader_ex.hpp"
#include "mzml.hpp"
#include "mzmlspectrum.hpp"
#include "scan_protocol.hpp"
#include "mzmlreader.hpp"
#include "datafile_factory.hpp"
#include <adacquire/constants.hpp>
#include <adportable/debug.hpp>
#include <adportable/bzip2.hpp>
#include <adutils/datafile_signature.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <net/if_var.h>

namespace mzml {
    class export_to_adfs::impl {
    public:
        std::shared_ptr< adfs::sqlite > db_;

        // 9C457C8F-D0B6-44EA-98F5-E89F8A229A63
        static constexpr const boost::uuids::uuid uuid_ =
        { 0x9C, 0x45, 0x7C, 0x8F, 0xD0, 0xB6, 0x44, 0xEA, 0x98, 0xF5, 0xE8, 0x9F, 0x8A, 0x22, 0x9A, 0x63 };
    };
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
    // Spectrometer uuid, scantype, description, fLength
    // AcquiredConf
    // AcquiredData (blob)
    // ScanLaw { uuid (observer id), objtext (observer text), acclVoltage, tDelay, spectrometer (text), spectrometer (clsid)
    // MetaData := ControlMethod clsid	'cd53abe6-8223-11e6-b2d8-cb9185077a24'

    adutils::data_signature::datafileSignature::create_table( *impl_->db_ );
    adutils::v3::AcquiredData::create_table_v3( *impl_->db_ );

    adfs::stmt sql( *impl_->db_ );

    using namespace adutils::data_signature;
    sql << datum_t{ "creator", value_t( impl_->uuid_ ) };
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
        ADDEBUG() << "run defaultInstrumentConfigurationRef="
                  << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
        ADDEBUG() << "\t=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
        ADDEBUG() << "\t=" << run.node().attribute( "defaultSourceFileRef" ).value();
        ADDEBUG() << "\t=" << run.node().attribute( "id" ).value();
    }
    const auto reader_uuid = mzml::local::data_reader::__uuid__();

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
            ADDEBUG() << scan_id
                      << "\tcomp.ratio=" << double(xml.size()) / xml.size();
        }
        scan_id_accessor scan_ident( scan_id );
        const uint32_t events = ( wcounts++ == 0 ) ?
            adacquire::SignalObserver::wkEvent::wkEvent_AcqInProgress | adacquire::SignalObserver::wkEvent::wkEvent_INJECT
            : adacquire::SignalObserver::wkEvent::wkEvent_AcqInProgress;
        if ( ! adutils::v3::AcquiredData::insert(
                 *impl_->db_
                 , reader_uuid
                 , uint64_t( scan_ident.scan_start_time() * 1e9 ) // elapsed_time
                 , uint64_t( scan_ident.scan_start_time() * 1e9 ) // epoch_time
                 , scan_ident.scan_index() // pos
                 , sp->protocol_id()   // fcn
                 , sp->length()    // ndata (number of data in the buffer
                 , events
                 , xdata  // xdata
                 , {}     // xmeta
                 ) ) {
        }


    }

    return true;
}
