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

#include "xchromatogramsmethod.hpp"
#include "tofchromatogrammethod.hpp"
#include "serializer.hpp"
#include "constants.hpp"
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/json.hpp>
#include <string>
#include <map>
#include <vector>

namespace adcontrols {

    namespace xic {

        xic_method::xic_method() : mol_{}
                                 , adduct_{}
                                 , mass_window_{}
                                 , time_window_{}
                                 , algo_{ ePeakAreaOnProfile }
                                 , protocol_{0}
        {
        }

        xic_method::xic_method( const xic_method& t ) : mol_( t.mol_ )
                                                      , adduct_( t.adduct_ )
                                                      , mass_window_( t.mass_window_ )
                                                      , time_window_( t.time_window_ )
                                                      , algo_( t.algo_ )
                                                      , protocol_( t.protocol_ )
        {
        }

        template<class Archive>
        inline void serialize( Archive & ar, xic_method& _, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( std::get< 0 >( _.mol_ ) ); // enable
            ar & BOOST_SERIALIZATION_NVP( std::get< 1 >( _.mol_ ) ); // synonym
            ar & BOOST_SERIALIZATION_NVP( std::get< 2 >( _.mol_ ) ); // formula
            ar & BOOST_SERIALIZATION_NVP( std::get< 3 >( _.mol_ ) ); // smiles
            ar & BOOST_SERIALIZATION_NVP( std::get< 0 >( _.adduct_ ) ); // polarity_positive
            ar & BOOST_SERIALIZATION_NVP( std::get< 1 >( _.adduct_ ) ); // polarity_negative
            ar & BOOST_SERIALIZATION_NVP( _.mass_window_ );
            ar & BOOST_SERIALIZATION_NVP( _.time_window_ );
            ar & BOOST_SERIALIZATION_NVP( _.algo_ );
            ar & BOOST_SERIALIZATION_NVP( _.protocol_ );
        }
    }

    ///////////////
    class XChromatogramsMethod::impl {
    public:
        impl() : numberOfTriggers_( 100 )
               , refreshHistogram_( true ) {
        }

        impl( const impl& t ) : numberOfTriggers_( t.numberOfTriggers_ )
                              , refreshHistogram_( t.refreshHistogram_ ){
        }

        constants::ion_polarity polarity_;
        size_t numberOfTriggers_;
        bool refreshHistogram_; // real-time monitoring parameter
        bool enableTIC_;
        xic::eIntensityAlgorishm algo_;
        std::vector< xic::xic_method > xics_;

    private:
        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( polarity_ );
            ar & BOOST_SERIALIZATION_NVP( numberOfTriggers_ );
            ar & BOOST_SERIALIZATION_NVP( refreshHistogram_ );
            ar & BOOST_SERIALIZATION_NVP( enableTIC_ );
            ar & BOOST_SERIALIZATION_NVP( algo_ );
            ar & BOOST_SERIALIZATION_NVP( xics_ );
        }
    };
}

BOOST_CLASS_VERSION( adcontrols::XChromatogramsMethod::impl, 1 )

using namespace adcontrols;

XChromatogramsMethod::~XChromatogramsMethod()
{
}

XChromatogramsMethod::XChromatogramsMethod() : impl_( std::make_unique< impl >() )
{
}

XChromatogramsMethod::XChromatogramsMethod( const XChromatogramsMethod& t ) : impl_( new impl( *t.impl_ ) )
{
}

size_t
XChromatogramsMethod::size() const
{
    return impl_->xics_.size();
}

void
XChromatogramsMethod::clear()
{
    impl_->xics_.clear();
}

constants::ion_polarity
XChromatogramsMethod::polarity() const
{
    return impl_->polarity_;
}

void
XChromatogramsMethod::setPolarity( constants::ion_polarity t )
{
    impl_->polarity_ = t;
}

size_t
XChromatogramsMethod::numberOfTriggers() const
{
    return impl_->numberOfTriggers_;
}

void
XChromatogramsMethod::setNumberOfTriggers( size_t value )
{
    impl_->numberOfTriggers_ = value;
}

bool
XChromatogramsMethod::refreshHistogram() const
{
    return impl_->refreshHistogram_;
}

void
XChromatogramsMethod::setRefreshHistogram( bool refresh )
{
    impl_->refreshHistogram_ = refresh;
}

std::tuple< bool, xic::eIntensityAlgorishm >
XChromatogramsMethod::tic() const
{
    return { impl_->enableTIC_, impl_->algo_ };
}

void
XChromatogramsMethod::setTIC( std::tuple< bool, xic::eIntensityAlgorishm >&& t )
{
    std::tie( impl_->enableTIC_, impl_->algo_ ) = std::move( t );
}

bool
XChromatogramsMethod::archive( std::ostream& os, const XChromatogramsMethod& t )
{
    return internal::binSerializer().archive( os, *t.impl_ );
}

//static
bool
XChromatogramsMethod::restore( std::istream& is, XChromatogramsMethod& t )
{
    return internal::binSerializer().restore( is, *t.impl_ );
}

//static
bool
XChromatogramsMethod::xml_archive( std::wostream& os, const XChromatogramsMethod& t )
{
    return internal::xmlSerializer("XChromatogramsMethod").archive( os, *t.impl_ );
}

//static
bool
XChromatogramsMethod::xml_restore( std::wistream& is, XChromatogramsMethod& t )
{
    return internal::xmlSerializer("XChromatogramsMethod").restore( is, *t.impl_ );
}

const boost::uuids::uuid&
XChromatogramsMethod::clsid()
{
    static const boost::uuids::uuid baseid = boost::uuids::string_generator()( "{6A4E59EB-0E9D-49B5-8812-3D6FC5084638}" );
    static const boost::uuids::uuid myclsid = boost::uuids::name_generator( baseid )( "adcontrols::XChromatogramsMethod" );
    return myclsid;
}

namespace adcontrols {

    namespace xic {
        void
        tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const xic_method& t )
        {
            jv = boost::json::object{{ "xic_method"
                    , {
                        { "mol",           t.mol_ }
                        , { "adduct",      t.adduct_ }
                        , { "mass_window", t.mass_window_ }
                        , { "time_window", t.time_window_ }
                        , { "algo",        static_cast< unsigned int>( t.algo_ ) }
                        , { "protocol",    t.protocol_ }
                    }
                }};
        }

        xic_method
        tag_invoke( boost::json::value_to_tag< xic_method >&, const boost::json::value& jv )
        {
            using namespace adportable::json;
            xic_method t;
            if ( jv.is_object() ) {
                if ( auto top = jv.as_object().if_contains( "xic_method" ) ) {
                    const auto& obj = top->as_object();
                    extract( obj, t.mol_,         "mol" );
                    extract( obj, t.adduct_,      "adduct" );
                    extract( obj, t.mass_window_, "mass_window" );
                    extract( obj, t.time_window_, "time_window" );
                    extract( obj, reinterpret_cast< unsigned int& >(t.algo_), "algo" );
                    extract( obj, t.protocol_,    "protocol" );
                }
            }
            return t;
        }
    }

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const XChromatogramsMethod& t )
    {
        jv = boost::json::object{{ "XICMethod"
                , {
                    { "numberOfTriggers",      t.impl_->numberOfTriggers_ }
                    , { "refreshHistogram",    t.impl_->refreshHistogram_ }
                    , { "enableTIC",           t.impl_->enableTIC_        }
                    , { "algoTIC",             static_cast<unsigned int>( t.impl_->algo_ )      }
                    , { "polarity",            static_cast<unsigned int>( t.impl_->polarity_ )  }
                    , { "xics",                t.impl_->xics_             }
                }
            }};
    }

    XChromatogramsMethod
    tag_invoke( boost::json::value_to_tag< XChromatogramsMethod >&, const boost::json::value& jv )
    {
        XChromatogramsMethod t;
        using namespace adportable::json;
        auto tv = adportable::json_helper::find( jv, "XICMethod" );
        if ( tv.is_object() ) {
            try {
                auto obj = tv.as_object();
                extract( obj, t.impl_->numberOfTriggers_, "numberOfTriggers" );
                extract( obj, t.impl_->refreshHistogram_, "refreshHistogram" );
                extract( obj, t.impl_->enableTIC_,        "enableTIC" );
                extract( obj, reinterpret_cast< unsigned int& >(t.impl_->algo_), "algoTIC" );
                extract( obj, reinterpret_cast< unsigned int& >(t.impl_->polarity_), "polarity" );
                extract( obj, t.impl_->xics_,              "xics" );
            } catch ( std::exception& ex ) {
                BOOST_THROW_EXCEPTION(std::runtime_error("adportable/json/extract<XChromatogramsmethod> exception"));
            }
        }
        return t;
    }
}
