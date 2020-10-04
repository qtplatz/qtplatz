/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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
#include <compiler/boost/workaround.hpp>
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

namespace adcontrols {
    namespace ControlMethod {

        class toValue : public boost::static_visitor< boost::property_tree::ptree > {
        public:
            boost::property_tree::ptree operator()( const duration_type& arg ) const {
                boost::property_tree::ptree value;
                value.put( "value", arg.value );
                value.put( "type", "duration_type" );
                value.put( "limits.min",  arg.limits.first );
                value.put( "limits.max",  arg.limits.second );
                return value;
            }
            boost::property_tree::ptree operator()( const voltage_type& arg ) const {
                boost::property_tree::ptree value;
                value.put( "type", "voltage_type" );
                value.put( "value", arg.value );
                value.put( "limits.min",  arg.limits.first );
                value.put( "limits.max",  arg.limits.second );
                return value;
            }
            boost::property_tree::ptree operator()( const switch_type& arg ) const {
                boost::property_tree::ptree value;
                value.put( "type", "switch_type" );
                value.put( "value", arg.value );
                return value;
            }
            boost::property_tree::ptree operator()( const choice_type& arg ) const {
                boost::property_tree::ptree value;
                value.put( "type", "choice_type" );
                value.put( "value", arg.value );
                boost::property_tree::ptree choices;
                for ( const auto& choice: arg.choice ) {
                    boost::property_tree::ptree a;
                    a.put( "choice", choice );
                    choices.push_back( { "", a } );
                }
                value.put_child( "choice_sequence", choices );
                return value;
            }
            boost::property_tree::ptree operator()( const delay_width_type& arg ) const {
                boost::property_tree::ptree value;
                value.put( "type", "delay_width_type" );
                value.put( "value.first", arg.value.first );
                value.put( "value.second", arg.value.second );
                return value;
            }
            boost::property_tree::ptree operator()( const any_type& arg ) const {
                boost::property_tree::ptree value;
                value.put( "type", "any_type" );
                value.put( "value", arg.value );
                value.put( "editor", arg.editor_ ); // uuid
                return value;
            }
        };
        //--------------------


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
                if ( version == 0 ) {
                    boost::uuids::uuid clsid = {{0}};
                    std::string module_display_name, item_name, item_display_name;
                    double time;
                    EventCap::value_type value;
                    ar & BOOST_SERIALIZATION_NVP( clsid );
                    ar & BOOST_SERIALIZATION_NVP( module_display_name );
                    ar & BOOST_SERIALIZATION_NVP( item_name );
                    ar & BOOST_SERIALIZATION_NVP( item_display_name );
                    ar & BOOST_SERIALIZATION_NVP( time );
                    ar & BOOST_SERIALIZATION_NVP( value );

                    ADDEBUG() << "clsid: " << clsid;
                    ADDEBUG() << "module_display_name: " << module_display_name;
                    ADDEBUG() << "item_name: " << item_name;
                    ADDEBUG() << "item_display_name: " << item_display_name;
                    ADDEBUG() << "time: " << time;
                    ADDEBUG() << "value: " << TimedEvent::toString( value );

                    _.ptree_->put( "modelClsid", clsid );
                    _.ptree_->put( "modelDisplayName", module_display_name );
                    _.ptree_->put( "name", item_name );
                    _.ptree_->put( "displayName", item_display_name );
                    _.ptree_->put( "time", time );
                    _.ptree_->put_child( "data", boost::apply_visitor( toValue(), value ) );
                }
                if ( version >= 1 ) {
                    ar & BOOST_SERIALIZATION_NVP( _.ptree_ );
                }
#if !defined NDEBUG
                std::ostringstream o;
                boost::property_tree::write_json( o, *_.ptree_ );
                ADDEBUG() << "\n" << o.str();
#endif
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

TimedEvent::TimedEvent() : ptree_( std::make_unique< boost::property_tree::ptree >() )
{
}

TimedEvent::TimedEvent( const TimedEvent& t ) : ptree_( std::make_unique< boost::property_tree::ptree >( *t.ptree_ ) )
{
}

TimedEvent::TimedEvent( const ModuleCap& moduleCap, const EventCap& eventCap, double time, const value_type& value )
{
    ptree_ = std::make_unique< boost::property_tree::ptree >();
    ptree_->put( "modelClsid", moduleCap.clsid() );
    ptree_->put( "modelDisplayName", moduleCap.model_display_name() );
    ptree_->put( "name", eventCap.item_name() );
    ptree_->put( "displayName", eventCap.item_display_name() );
    ptree_->put( "time", time );

    auto values = boost::apply_visitor( toValue(), value );
    ptree_->put_child( "data", values );

#if !defined NDEBUG
    std::ostringstream o;
    boost::property_tree::write_json( o, *ptree_ );
    ADDEBUG() << "\n" << o.str();
#endif

}

TimedEvent::TimedEvent( const boost::property_tree::ptree& pt ) : ptree_( std::make_unique< boost::property_tree::ptree >( pt ) )
{
}

TimedEvent::TimedEvent( boost::property_tree::ptree&& pt ) : ptree_( std::make_unique< boost::property_tree::ptree >( std::move( pt ) ) )
{
}

TimedEvent&
TimedEvent::operator = ( const TimedEvent& t )
{
    ptree_ = std::make_unique< boost::property_tree::ptree >( *t.ptree_ );
    return *this;
}


TimedEvent::operator bool () const // is valid for json
{
    if ( auto uuid = ptree_->get_optional< boost::uuids::uuid >( "modelClsid" ) )
        return uuid != boost::uuids::uuid( {{ 0 }} );
    return true;
}

double
TimedEvent::time() const
{
    if ( auto time = ptree_->get_optional< double >( "time" ) )
        return *time;
    return 0;
}

void
TimedEvent::setTime( double seconds )
{
    ptree_->put( "time", seconds );
}

void
TimedEvent::setModelClsid( const boost::uuids::uuid& uuid )
{
    ptree_->put( "moduleClsid", uuid );
}

boost::uuids::uuid
TimedEvent::modelClsid() const
{
    if ( auto uuid = ptree_->get_optional< boost::uuids::uuid >( "modelClsid" ) )
        return *uuid;
    return {{0}};
}


std::string
TimedEvent::modelDisplayName() const
{
    if ( auto name = ptree_->get_optional< std::string >( "modelDisplayName" ) )
        return *name;
    return {};
}

TimedEvent::value_type
TimedEvent::value() const
{
    ADDEBUG() << "----------- value ------------";
    if ( auto typ = ptree_->get_optional< std::string >( "data.type" ) ) {
        if ( *typ == "any_type" ) {
            if ( auto value = ptree_->get_optional< std::string >( "data.value" ) ) {
                if ( auto editor = ptree_->get_optional< boost::uuids::uuid > ( "data.editor" ) ) {
                    return any_type( std::move( *value ), *editor );
                }
            }
        }
        if ( *typ == "duration_type" ) {
            if ( auto value = ptree_->get_optional< double >( "data.value" ) ) {
                auto min = ptree_->get_optional< double >( "data.limits.min" );
                auto max = ptree_->get_optional< double >( "data.limits.max" );
                if ( min && max )
                    return duration_type( *value, { *min, *max } );
            }
        }
        if ( *typ == "voltage_type" ) {
            if ( auto value = ptree_->get_optional< double >( "data.value" ) ) {
                auto min = ptree_->get_optional< double >( "data.limits.min" );
                auto max = ptree_->get_optional< double >( "data.limits.max" );
                if ( min && max )
                    return voltage_type( *value, { *min, *max } );
            }
        }
        if ( *typ == "switch_type" ) {
            if ( auto value = ptree_->get_optional< bool >( "data.value" ) ) {
                return switch_type( *value );
            }
        }
        if ( *typ == "choice_type" ) {
            if ( auto value = ptree_->get_optional< int32_t >( "data.value" ) ) {
                if ( auto choices = ptree_->get_child_optional( "choice_sequence" ) ) {
                    assert(0);  // tba
                }
            }
        }
        if ( *typ == "delay_width_type" ) {
            assert(0);  // tba
        }

    }
    return EventCap::value_type{};
}

void
TimedEvent::setValue( const value_type& value )
{
    auto values = boost::apply_visitor( toValue(), value );
    ptree_->put_child("values", values );
}

std::string
TimedEvent::itemName() const
{
    if ( auto item_name = ptree_->get_optional< std::string >( "item_name" ) )
        return *item_name;
    return "";
}

void
TimedEvent::setItemName( const std::string& name )
{
    ptree_->put( "name", name );
}


std::string
TimedEvent::itemDisplayName() const
{
    if ( auto name = ptree_->get_optional< std::string >( "displayName" ) )
        return *name;
    return {};
}

void
TimedEvent::setItemDisplayName( const std::string& name )
{
    ptree_->put( "displayName", name );
}

std::string
TimedEvent::data_type() const
{
    if ( auto typ = ptree_->get_optional< std::string >( "type" ) )
        return *typ;
    return "";
}

//static
std::string
TimedEvent::toString( const value_type& v )
{
    return EventCap::toString( v );
}

boost::property_tree::ptree *
TimedEvent::ptree()
{
    return ptree_.get();
}

const boost::property_tree::ptree *
TimedEvent::ptree() const
{
    return ptree_.get();
}

std::string
TimedEvent::json() const
{
    std::ostringstream o;
    boost::property_tree::write_json( o, *ptree_ );
    return o.str();
}
