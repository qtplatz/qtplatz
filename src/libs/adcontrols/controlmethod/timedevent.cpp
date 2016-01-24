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

#include "timedevent.hpp"
#include "serializer.hpp"
#include "modulecap.hpp"
#include "eventcap.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace adcontrols {
    namespace ControlMethod {

        template<class Archive>
        void serialize( Archive& ar, adcontrols::ControlMethod::elapsed_time_type& _, const unsigned int ) {
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
            ar & BOOST_SERIALIZATION_NVP( _.value );
        }

        template< typename T = TimedEvent >
        class TimedEvent_archive {
        public:
            friend class boost::serialization::access;
            template<class Archive> void serialize( Archive& ar, T& _, const unsigned int version )
            {
                using namespace boost::serialization;

                ar & BOOST_SERIALIZATION_NVP( _.clsid_ );
                ar & BOOST_SERIALIZATION_NVP( _.module_display_name_ );
                ar & BOOST_SERIALIZATION_NVP( _.item_name_ );
                ar & BOOST_SERIALIZATION_NVP( _.item_display_name_ );
                ar & BOOST_SERIALIZATION_NVP( _.time_ );
                ar & BOOST_SERIALIZATION_NVP( _.value_ );
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

TimedEvent::TimedEvent() : clsid_( { 0 } )
                         , time_( 0 )
{
}

TimedEvent::TimedEvent( const TimedEvent& t ) : clsid_( t.clsid_ )
                                              , module_display_name_( t.module_display_name_ )
                                              , item_name_( t.item_name_ )
                                              , item_display_name_( t.item_display_name_ )
                                              , time_( t.time_ )
                                              , value_( t.value_ )
{
}

TimedEvent::TimedEvent( const ModuleCap& moduleCap, const EventCap& eventCap, double time, const value_type& value )
{
    clsid_ = moduleCap.clsid();
    module_display_name_ = moduleCap.model_display_name();
    item_name_ = eventCap.item_name();
    item_display_name_ = eventCap.item_display_name();
    time_ = time;
    value_ = value;
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
    clsid_ = uuid;
}

const boost::uuids::uuid&
TimedEvent::modelClsid() const
{
    return clsid_;
}

const TimedEvent::value_type&
TimedEvent::value() const
{
    return value_;
}

void
TimedEvent::setValue( const value_type& value )
{
    value_ = value;
}

const std::string&
TimedEvent::item_name() const
{
    return item_name_;
}

void
TimedEvent::setItem_name( const std::string& name )
{
    item_name_ = name;
}


const std::string&
TimedEvent::item_display_name() const
{
    return item_display_name_;
}

void
TimedEvent::setItem_display_name( const std::string& name )
{
    item_display_name_ = name;
}

