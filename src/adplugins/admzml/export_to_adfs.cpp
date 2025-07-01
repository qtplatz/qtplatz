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
#include "mzml.hpp"
#include "mzmlspectrum.hpp"
#include "scan_protocol.hpp"
#include "mzmlreader.hpp"
#include <adportable/debug.hpp>
#include <adportable/bzip2.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <sstream>

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

    adfs::stmt sql( *impl_->db_ );

    if ( auto d = _.get_fileDescription() ) {
        ADDEBUG() << boost::json::value_from( *d );
    }
    if ( auto d = _.get_softwareList() ) {
        ADDEBUG() << boost::json::value_from( *d );
    }
    if ( auto d = _.get_instrumentConfigurationList() ) {
        ADDEBUG() << boost::json::value_from( *d );
    }
    if ( auto d = _.get_dataProcessingList() ) {
        std::ostringstream o;
        d->node().print( o );
        ADDEBUG() << o.str();
    }

    if ( auto run = _.xml_document().select_node( "/indexdmzML/mzML/run" ) ) {
        ADDEBUG() << "run defaultInstrumentConfigurationRef="
                  << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
        ADDEBUG() << "\t=" << run.node().attribute( "defaultInstrumentConfigurationRef" ).value();
        ADDEBUG() << "\t=" << run.node().attribute( "defaultSourceFileRef" ).value();
        ADDEBUG() << "\t=" << run.node().attribute( "id" ).value();
    }

    for ( const auto [scan_id,sp]: _.scan_indices() ) {
        // scan_id = std::tuple< int, string, double (time), scan_protocol
        // sp = mzMLSpectrum
        auto xml = sp->serialize();
        std::string ar;
        adportable::bzip2::compress( ar, xml.data(), xml.size() );

        { // verify
            std::string inflated;
            adportable::bzip2::decompress( inflated, ar.data(), ar.size() );
            auto spc = serializer::deserialize( inflated.data(), inflated.size() );

            ADDEBUG() << scan_id
                      << "\t size=" << std::make_pair( ar.size(), xml.size() );
        }

    }

    return true;
}
