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

#include "generator_property.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <boost/json.hpp>

namespace adprocessor {

    class generator_property::impl {
    public:
        boost::json::value jv_;
        std::string generator_;
        double mass_;
        std::optional< std::string > formula_;

        impl( const adcontrols::Chromatogram& c ) : mass_( 0 ) {

            jv_ = adportable::json_helper::parse( c.generatorProperty() );

            if ( auto gen = adportable::json_helper::if_contains( jv_, "generator.extract_by_peak_info" ) ) {
                generator_ = "extract_by_peak_info"; // gen from mass peak
                if ( auto value = adportable::json_helper::if_contains( *gen, "pkinfo.mass" ) )
                    mass_ = value->as_double();
            } else if (  auto gen = adportable::json_helper::if_contains( jv_, "generator.extract_by_mols" ) ) {
                generator_ = "extract_by_mols";  // gen from mschromatogr. parameter
                if ( auto value = adportable::json_helper::if_contains( *gen, "moltable.mass" ) )
                    mass_ = value->as_double();
                if ( auto value = adportable::json_helper::if_contains( *gen, "moltable.formula" ) )
                    formula_ = value->as_string();
            }
        }
    };

    generator_property::~generator_property()
    {
        delete impl_;
    }

    generator_property::generator_property( const adcontrols::Chromatogram& c ) : impl_( new impl( c ) )
    {
    }

    std::string
    generator_property::generator() const
    {
        return impl_->generator_;
    }

    std::optional< std::string > generator_property::formula() const
    {
        return impl_->formula_;
    }

    double
    generator_property::mass() const
    {
        return impl_->mass_;
    }

    std::tuple< double, std::string, std::string >
    generator_property::get() const
    {
        return { impl_->mass_
                 , impl_->formula_ ? *impl_->formula_ : ""
                 , impl_->generator_ };
    }

}
