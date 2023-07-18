/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "jcb2009processor.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/peaks.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adportfolio/folium.hpp>

#include <boost/json.hpp>
#include <memory>

namespace adprocessor {

    class JCB2009Processor::impl {
    public:
        std::shared_ptr< adprocessor::dataprocessor > processor_;
        std::vector< portfolio::Folium > folio_;

        impl( adprocessor::dataprocessor * dp )
            : processor_( dp->shared_from_this() ) {
        }
    };
}

using namespace adprocessor;

JCB2009Processor::~JCB2009Processor()
{
}

JCB2009Processor::JCB2009Processor( adprocessor::dataprocessor * processor )
    : impl_( std::make_unique< impl >( processor ) )
{
}

void
JCB2009Processor::operator << ( portfolio::Folium&& folium )
{
    impl_->folio_.emplace_back( std::move( folium ) );

    const auto& xfolium = impl_->folio_.back();
    if ( auto chro = portfolio::get< adcontrols::ChromatogramPtr >( xfolium ) ) {
        ADDEBUG() << xfolium.name() << "\tpeaks.size: " << chro->peaks().size();

        auto jv = adportable::json_helper::parse( chro->generatorProperty() );
        if ( jv.is_object() ) {
            if ( auto gen = jv.as_object().if_contains( "generator" ) ) {
                if ( auto value = gen->as_object().if_contains( "extract_by_peak_info" ) ) {
                    auto mv = adportable::json_helper::find( *value, "pkinfo.mass" );
                    ADDEBUG() << "extract_by_peak_info: mass = " << mv.as_double();
                    } else if ( auto value = gen->as_object().if_contains( "extract_by_mols" ) ) {
                    ADDEBUG() << "extract_by_mols: " << adportable::json_helper::find( *value, "moltable" );
                } else if ( auto value = gen->as_object().if_contains( "extract_by_axis_range" ) ) {
                    ADDEBUG() << "extract_by_axis_range: " << *value;
                }
            }
        }
    }
}
