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
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <boost/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <regex>

namespace adprocessor {

    class generator_property::impl {
    public:
        boost::json::value jv_;
        std::string generator_;
        double mass_;
        double mass_width_;
        int proto_;
        std::optional< std::string > formula_;
        std::string reader_name_;
        std::pair< std::string, boost::uuids::uuid > dataSource_;

        impl() : mass_( 0 )
               , mass_width_( 0 )
               , dataSource_( { "", boost::uuids::uuid{} } ) {
        }

        impl( const impl& t ) : jv_( t.jv_ )
                              , generator_( t.generator_ )
                              , mass_( t.mass_ )
                              , mass_width_( t.mass_width_ )
                              , proto_( t.proto_ )
                              , formula_( t.formula_ )
                              , reader_name_( t.reader_name_ )
                              , dataSource_( t.dataSource_ ) {
        }

        impl( const adcontrols::Chromatogram& c ) : mass_( 0 )
                                                  , mass_width_( 0 ) {

            jv_ = adportable::json_helper::parse( c.generatorProperty() );
            setup( jv_ );

            if ( auto value = adportable::json_helper::if_contains( jv_, "mass_width" ) ) {
                mass_width_ = value->as_double();
            }
            if ( mass_ == 0 ) {
                if ( auto value = adportable::json_helper::if_contains( jv_, "mass" ) )
                    mass_ = value->as_double();
            }
            if ( auto value = adportable::json_helper::if_contains( jv_, "protocol" ) )
                proto_ = value->as_int64();

            if ( auto value = adportable::json_helper::if_contains( jv_, "reader" ) )
                reader_name_ = value->as_string();

            // workaround
            if ( mass_width_ == 0 ) {
                for ( const auto& desc: c.descriptions() ) {
                    auto [key,text] = desc.keyValue();
                    std::smatch match;
                    if ( std::regex_match( text, match
                                           , std::regex( R"__(.*\(W[ :]*([0-9]+\.[0-9]+)mDa\).*|.*\(W[ :]*([0-9]+)mDa\).*)__" ) ) ) {
                        if ( match.size() == 3 ) {
                            mass_width_ = std::stod( match[1].str().empty() ? match[2].str() : match[1].str() ) / 1000.0;
                            break;
                        }
                    }
                }
            }
            if ( reader_name_.empty() ) {
                for ( const auto& desc: c.descriptions() ) {
                    auto [key,text] = desc.keyValue();
                    std::smatch match;
                    if ( std::regex_match( text, match
                                           , std::regex( R"__(.*\(W[ :]*[0-9\.]+mDa\)[ ,]*([a-zA-Z]*).*([0-9]))__" ) ) ) {
                        if ( match.size() == 3 ) {
                            reader_name_ = match[1].str();
                            proto_ = std::stoi( match[2].str() );
                        }
                        if ( !reader_name_.empty() )
                            std::transform( reader_name_.begin(), reader_name_.end(), reader_name_.begin(), ::toupper );
                    }
                }
            }
        }
        void setup( const boost::json::value& jv ) {
            if ( auto gen = adportable::json_helper::if_contains( jv, "generator.extract_by_peak_info" ) ) {
                generator_ = "extract_by_peak_info"; // gen from mass peak
                if ( auto value = adportable::json_helper::if_contains( *gen, "pkinfo.mass" ) )
                    mass_ = value->as_double();
            } else if (  auto gen = adportable::json_helper::if_contains( jv, "generator.extract_by_mols" ) ) {
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
    }

    generator_property::generator_property() : impl_( std::make_unique< impl >() )
    {
    }

    generator_property::generator_property( const generator_property& t )
        : impl_( std::make_unique< impl >( *t.impl_ ) )
    {
    }

    generator_property&
    generator_property::operator = ( const generator_property& t )
    {
        impl_ = std::make_unique< impl >( *t.impl_ );
        return *this;
    }

    generator_property::generator_property( const adcontrols::Chromatogram& c ) : impl_( std::make_unique< impl >( c ) )
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

    double
    generator_property::mass_width() const
    {
        return impl_->mass_width_;
    }

    const std::string&
    generator_property::data_reader() const
    {
        return impl_->reader_name_;
    }

    int
    generator_property::protocol() const
    {
        return impl_->proto_;
    }

    void
    generator_property::set_dataSource( std::pair< std::string, boost::uuids::uuid >&& t )
    {
        impl_->dataSource_ = std::move( t );
    }

    std::pair< std::string, boost::uuids::uuid >
    generator_property::dataSource() const
    {
        return impl_->dataSource_;
    }


    std::tuple< double, std::string, std::string >
    generator_property::get() const
    {
        return { impl_->mass_
                 , impl_->formula_ ? *impl_->formula_ : ""
                 , impl_->generator_ };
    }

    const boost::json::value&
    generator_property::value() const
    {
        return impl_->jv_;
    }

    ////////////////////////////////////////////////

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const generator_property& t )
    {
        using namespace adportable;
        auto gen = t.impl_->jv_.as_object().if_contains( "generator" );
        jv = {
            { "mass",         t.impl_->mass_ }
            , { "mass_width", t.impl_->mass_width_ }
            , { "protocol",   t.impl_->proto_ }
            , { "reader",     t.impl_->reader_name_ }
            , { "dataSource", {{ "folder_name", t.impl_->dataSource_.first,  }
                               , { "folder_uuid", boost::uuids::to_string(t.impl_->dataSource_.second) }} }
            , { "generator",  gen ? *gen : boost::json::object{} }
        };
    }

    generator_property
    tag_invoke( boost::json::value_to_tag< generator_property >&, const boost::json::value& jv )
    {
        using namespace adportable::json;

        if ( jv.kind() == boost::json::kind::object ) {
            generator_property t;
            auto obj = jv.as_object();
            extract( obj, t.impl_->mass_,        "mass" );
            extract( obj, t.impl_->mass_width_,  "mass_width" );
            extract( obj, t.impl_->proto_,       "protocol" );
            extract( obj, t.impl_->reader_name_, "reader" );
            // extract( obj, t.impl_->dataSource_,  "dataSource" );
            if ( auto p = obj.if_contains( "generator" ) ) {
                t.impl_->jv_ = {{ "generator", *p }};
                t.impl_->setup( t.impl_->jv_ );
            }
            if ( auto p = obj.if_contains( "dataSource" ) ) {
                extract( p->as_object(), t.impl_->dataSource_.first, "folder_name" );
                extract( p->as_object(), t.impl_->dataSource_.second, "folder_uuid" );
            }
            return t;
        }
        return {};
    }

}
