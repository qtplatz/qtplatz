/**************************************************************************
** Copyright (C) 2010-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2022 MS-Cheminformatics LLC
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

#include "timedevent.hpp"
#include "serializer.hpp"
#include "modulecap.hpp"
#include "eventcap.hpp"
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree_serialization.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/variant.hpp>
#include <boost/json.hpp>
#include <boost/exception/all.hpp>

namespace adcontrols {
    namespace ControlMethod {

        struct null_type {};
        template< typename... Types> struct value_type_list {};
        using value_types = value_type_list< duration_type, voltage_type
                                             , switch_type, choice_type
                                             , delay_width_type, any_type
                                             , null_type >;

        template< typename last_t > struct value_type_list< last_t > {
        };
        template< typename first_t, typename... args > struct value_type_list< first_t, args ... > {
            std::string name() const {
                return value_type_list< args ... >().name();
            }
        };


        class value_name : public boost::static_visitor< std::string > {
        public:
            std::string operator()( const duration_type& ) const    { return "duration_type"; }
            std::string operator()( const voltage_type& ) const     { return "voltage_type"; }
            std::string operator()( const switch_type& ) const      { return "switch_type"; }
            std::string operator()( const choice_type& ) const      { return "choice_type"; }
            std::string operator()( const delay_width_type& ) const { return "delay_width_type"; }
            std::string operator()( const any_type& ) const         { return "any_type"; }
            template< typename T > std::string operator()( const T& ) const { return "unknown_type"; };
        };

        class toValue : public boost::static_visitor< boost::json::value > {
        public:
            boost::json::value operator()( const duration_type& arg ) const {
                return boost::json::object{
                    { "type", value_name()( arg ) }
                    , { "value", arg.value }
                    , { "limits.min",  arg.limits.first }
                    , { "limits.max",  arg.limits.second }
                };
            }
            boost::json::value operator()( const voltage_type& arg ) const {
                return boost::json::object{
                    { "type", value_name()( arg ) }
                    , { "value", arg.value }
                    , { "limits.min",  arg.limits.first }
                    , { "limits.max",  arg.limits.second }
                };
            }
            boost::json::value operator()( const switch_type& arg ) const {
                return boost::json::object{
                    { "type", value_name()( arg ) }
                    , { "value", arg.value }
                };
            }
            boost::json::value operator()( const choice_type& arg ) const {
                return boost::json::object{
                    { "type", value_name()( arg ) }
                    , { "value", arg.value }
                    , { "choice_sequence", boost::json::value_from( arg.choice ) }
                };
            }
            boost::json::value operator()( const delay_width_type& arg ) const {
                return boost::json::object{
                    { "type", value_name()( arg ) }
                    , { "value.first", arg.value.first }
                    , { "value.second", arg.value.second }
                };
            }
            boost::json::value operator()( const any_type& arg ) const {
                return boost::json::object{
                    { "type", value_name()( arg ) }
                    , { "value", arg.value }
                    , { "editor", arg.editor_ } // uuid
                };
            }
        };
        //--------------------
        using namespace adportable::json;
        using namespace adportable;
        duration_type tag_invoke( boost::json::value_to_tag< duration_type >&, const boost::json::value& jv ) {
            duration_type t;
            if ( value_name()( t ) != boost::json::value_to< std::string >( json_helper::find( jv, "type" ) ) )
                BOOST_THROW_EXCEPTION( std::runtime_error( "type mismatch" ) );

            extract( jv.as_object(), t.value, "value" );
            auto limits = json_helper::find( jv, "limits" );
            if ( limits.is_object() ) {
                extract( limits.as_object(), t.limits.first, "min" );
                extract( limits.as_object(), t.limits.first, "max" );
            }
            return t;
        }
        voltage_type  tag_invoke( boost::json::value_to_tag< voltage_type >&, const boost::json::value& jv ) {
            voltage_type t;
            if ( value_name()( t ) != boost::json::value_to< std::string >( json_helper::find( jv, "type" ) ) )
                BOOST_THROW_EXCEPTION( std::runtime_error( "type mismatch" ) );

            extract( jv.as_object(), t.value, "value" );
            auto limits = json_helper::find( jv, "limits" );
            if ( limits.is_object() ) {
                extract( limits.as_object(), t.limits.first, "min" );
                extract( limits.as_object(), t.limits.first, "max" );
            }
            return t;
        }
        switch_type tag_invoke( boost::json::value_to_tag< switch_type >&, const boost::json::value& jv ) {
            switch_type t;
            if ( value_name()( t ) != boost::json::value_to< std::string >( json_helper::find( jv, "type" ) ) )
                BOOST_THROW_EXCEPTION( std::runtime_error( "type mismatch" ) );

            extract( jv.as_object(), t.value, "value" );
            return t;
        }
        choice_type tag_invoke( boost::json::value_to_tag< choice_type >&, const boost::json::value& jv ) {
            choice_type t;
            if ( value_name()( t ) != boost::json::value_to< std::string >( json_helper::find( jv, "type" ) ) )
                BOOST_THROW_EXCEPTION( std::runtime_error( "type mismatch" ) );
            extract( jv.as_object(), t.choice, "choice_sequence" ); // std;:vector< std::string >
            extract( jv.as_object(), t.value, "value" );
            return t;
        }
        delay_width_type tag_invoke( boost::json::value_to_tag< delay_width_type >&, const boost::json::value& jv ) {
            delay_width_type t;
            if ( value_name()( t ) != boost::json::value_to< std::string >( json_helper::find( jv, "type" ) ) )
                BOOST_THROW_EXCEPTION( std::runtime_error( "type mismatch" ) );
            auto value = json_helper::find( jv, "value" );
            extract( value.as_object(), t.value.first, "first" );
            extract( value.as_object(), t.value.second, "second" );
            return t;
        }
        any_type tag_invoke( boost::json::value_to_tag< any_type >&, const boost::json::value& jv ) {
            any_type t;
            if ( value_name()( t ) != boost::json::value_to< std::string >( json_helper::find( jv, "type" ) ) )
                BOOST_THROW_EXCEPTION( std::runtime_error( "type mismatch" ) );
            extract( jv.as_object(), t.value, "value" );
            extract( jv.as_object(), t.editor_, "editor" ); // uuid
            return t;
        }

        //============================================================================================
        //============================================================================================
        template<class Archive>
        void serialize( Archive& ar, adcontrols::ControlMethod::duration_type& _, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( _.value );
            ar & BOOST_SERIALIZATION_NVP( _.limits );
        }
        template<class Archive>
        void serialize( Archive& ar, adcontrols::ControlMethod::voltage_type& _, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( _.value);
            ar & BOOST_SERIALIZATION_NVP( _.limits );
        }
        template<class Archive>
        void serialize( Archive& ar, adcontrols::ControlMethod::switch_type& _, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( _.value );
            ar & BOOST_SERIALIZATION_NVP( _.choice );
        }
        template<class Archive>
        void serialize( Archive& ar, adcontrols::ControlMethod::choice_type& _, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( _.value );
            ar & BOOST_SERIALIZATION_NVP( _.choice );
        }
        template<class Archive>
        void serialize( Archive& ar, adcontrols::ControlMethod::delay_width_type& _, const unsigned int ) {
            ar & BOOST_SERIALIZATION_NVP( _.value );
        }
        template<class Archive>
        void serialize( Archive& ar, adcontrols::ControlMethod::any_type& _, const unsigned int ) {
            try {
                ar & BOOST_SERIALIZATION_NVP( _.editor_ );
                ar & BOOST_SERIALIZATION_NVP( _.value );
            } catch ( ... ) {}
        }


        template< typename T = TimedEvent >
        class TimedEvent_archive {
        public:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, T& _, const unsigned int version )
            {
                using namespace boost::serialization;
                if ( version == 1 && Archive::is_loading::value ) {
                    boost::property_tree::ptree ptree;
                    ar & BOOST_SERIALIZATION_NVP( ptree );
                    std::ostringstream o;
                    boost::property_tree::write_json( o, ptree );
                    auto jv = boost::json::parse( o.str() );
                    ADDEBUG() << jv;
                    _ = boost::json::value_to< TimedEvent >( jv );
                }
                if ( version == 0 || version >= 2 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.modelClsid_ );
                    ar & BOOST_SERIALIZATION_NVP( _.modelDisplayName_ );
                    ar & BOOST_SERIALIZATION_NVP( _.name_ );
                    ar & BOOST_SERIALIZATION_NVP( _.displayName_ );
                    ar & BOOST_SERIALIZATION_NVP( _.time_ );
                    ar & BOOST_SERIALIZATION_NVP( _.value_ );
                }
            }
        };

        ////////// PORTABLE BINARY ARCHIVE //////////
        template<> void
        TimedEvent::serialize( portable_binary_oarchive& ar, const unsigned int version )
        {
            TimedEvent_archive<>().serialize( ar, *this, version );
        }

        template<> void
        TimedEvent::serialize( portable_binary_iarchive& ar, const unsigned int version )
        {
            TimedEvent_archive<>().serialize( ar, *this, version );
        }

        ///////// XML archive ////////
        template<> void
        TimedEvent::serialize( boost::archive::xml_woarchive& ar, const unsigned int version )
        {
            TimedEvent_archive<>().serialize( ar, *this, version );
        }

        template<> void
        TimedEvent::serialize( boost::archive::xml_wiarchive& ar, const unsigned int version )
        {
            TimedEvent_archive<>().serialize( ar, *this, version );
        }
    }
}

using namespace adcontrols::ControlMethod;

TimedEvent::~TimedEvent()
{
}

TimedEvent::TimedEvent()
{
}

TimedEvent::TimedEvent( const TimedEvent& t )
    : modelClsid_( t.modelClsid_ )
    , modelDisplayName_( t.modelDisplayName_ )
    , name_ ( t.name_ ) // item_name
    , displayName_( t.displayName_ ) // item_display_name_; // item_display_name_
    , time_( t.time_ )
    , value_( t.value_ )
{
}

TimedEvent::TimedEvent( const ModuleCap& moduleCap, const EventCap& eventCap, double time, const value_type& value )
{
    modelClsid_ = moduleCap.clsid();
    modelDisplayName_ = moduleCap.model_display_name();
    name_ = eventCap.item_name();
    displayName_ = eventCap.item_display_name();
    time_ = time;
    value_ = value;
}

TimedEvent::TimedEvent( const std::string& json ) // : ptree_( std::make_unique< boost::property_tree::ptree >() )
{
    auto jv = boost::json::parse( json );
    *this = boost::json::value_to< TimedEvent >( jv );
}

TimedEvent&
TimedEvent::operator = ( const TimedEvent& t )
{
    modelClsid_       = t.modelClsid_;
    modelDisplayName_ = t.modelDisplayName_;
    name_             = t.name_;
    displayName_      = t.displayName_;
    time_             = t.time_;
    value_            = t.value_;
    return *this;
}


TimedEvent::operator bool () const // is valid for json
{
    return true;
}

double
TimedEvent::time() const
{
    return time_;
}

void
TimedEvent::setTime( double seconds )
{
    time_ = seconds;
}

void
TimedEvent::setModelClsid( const boost::uuids::uuid& uuid )
{
    modelClsid_ = uuid;
}

boost::uuids::uuid
TimedEvent::modelClsid() const
{
    return modelClsid_;
}


std::string
TimedEvent::modelDisplayName() const
{
    return modelDisplayName_;
}

TimedEvent::value_type
TimedEvent::value() const
{
    return value_;
}

void
TimedEvent::setValue( const value_type& value )
{
    value_ = value;
}

std::string
TimedEvent::itemName() const
{
    return name_;
}

void
TimedEvent::setItemName( const std::string& name )
{
    name_ = name;
}


std::string
TimedEvent::itemDisplayName() const
{
    return displayName_;
}

void
TimedEvent::setItemDisplayName( const std::string& name )
{
    displayName_ = name;
}

std::string
TimedEvent::data_type() const
{
    return value_name()( value_ );
}

//static
std::string
TimedEvent::toString( const value_type& v )
{
    return EventCap::toString( v );
}

namespace adcontrols {
    namespace ControlMethod {
        void
        tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const TimedEvent& t )
        {
            jv = {
                { "modelClsid", t.modelClsid_ }
                , { "modelDisplayName", t.modelDisplayName_ }
                , { "name", t.name_ }
                , { "displayName", t.displayName_ }
                , { "time", t.time_ }
                , { "data", boost::apply_visitor( toValue(), t.value_ ) }
            };
        }


        TimedEvent tag_invoke( boost::json::value_to_tag< TimedEvent >&, const boost::json::value& jv )
        {
            TimedEvent t;
            if ( jv.is_object() ) {
                using namespace adportable::json;
                const auto& obj = jv.as_object();

                extract( obj, t.modelClsid_, "modelClsid" );
                extract( obj, t.modelDisplayName_, "modelDisplayName" );
                extract( obj, t.name_, "name" );
                extract( obj, t.displayName_, "displayName" );
                extract( obj, t.time_, "time" );
                auto data = adportable::json_helper::find( obj, "data" );
                if ( data.is_object() ) {
                    auto type_name = boost::json::value_to< std::string >( data.as_object().at( "type" ) );
                    if ( type_name == value_name()( duration_type() ) ) {
                        t.value_ = boost::json::value_to< duration_type >( data );
                    } else if ( type_name == value_name()( voltage_type() ) ) {
                        t.value_ = boost::json::value_to< voltage_type >( data );
                    } else if ( type_name == value_name()( switch_type() ) ) {
                        t.value_ = boost::json::value_to< switch_type >( data );
                    } else if ( type_name == value_name()( choice_type() ) ) {
                        t.value_ = boost::json::value_to< choice_type >( data );
                    } else if ( type_name == value_name()( delay_width_type() ) ) {
                        t.value_ = boost::json::value_to< delay_width_type >( data );
                    } else if ( type_name == value_name()( any_type() ) ) {
                        t.value_ = boost::json::value_to< any_type >( data );
                    }
                }
            }
            return t;
        }
    }
}
