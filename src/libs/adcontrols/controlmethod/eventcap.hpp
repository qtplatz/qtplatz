/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#pragma once

#include "../adcontrols_global.h"
#include <boost/variant.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/format.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <boost/json/value_from.hpp>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace adcontrols {

    namespace ControlMethod {
        // xxx_type classes are serialized by adcontrols::ControlMethod::TimedEvent class in timedevent.cpp.
        // If more xxx_types are added, also add serializer in timedevent.cpp

        // -------------------------------------------
        struct ADCONTROLSSHARED_EXPORT voltage_type {
            double value;
            std::pair< double, double > limits;
            voltage_type( double _1 = 0, std::pair<double, double>&& _2 = { 0,0 } ) : value( _1 ), limits( _2 ) {}
            voltage_type( const voltage_type& t ) : value( t.value ), limits( t.limits ) {}
        };
        ADCONTROLSSHARED_EXPORT bool operator==(const voltage_type& lhs, const voltage_type& rhs);
        ADCONTROLSSHARED_EXPORT bool operator<(const voltage_type& lhs, const voltage_type& rhs);
        ADCONTROLSSHARED_EXPORT voltage_type  tag_invoke( const boost::json::value_to_tag< voltage_type >&, const boost::json::value& jv );
        ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const voltage_type& );

        // -------------------------------------------
        struct ADCONTROLSSHARED_EXPORT duration_type {
            double value;
            std::pair< double, double > limits;
            duration_type( double _1 = 0, std::pair<double, double>&& _2 = { 0,0 } ) : value( _1 ), limits( _2 ) {}
            duration_type( const duration_type& t ) : value( t.value ), limits( t.limits ) {}
        };
        ADCONTROLSSHARED_EXPORT bool operator==(const duration_type& lhs, const duration_type& rhs);
        ADCONTROLSSHARED_EXPORT bool operator<(const duration_type& lhs, const duration_type& rhs);
        ADCONTROLSSHARED_EXPORT duration_type tag_invoke( const boost::json::value_to_tag< duration_type >&, const boost::json::value& jv );
        ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const duration_type& );

        // -------------------------------------------
        struct ADCONTROLSSHARED_EXPORT switch_type {
            bool value;
            std::pair< std::string, std::string > choice;
            switch_type( bool _1 = false, std::pair<std::string, std::string>&& _2 = { "ON", "OFF" } ) : value( _1 ), choice( _2 ) {}
            switch_type( const switch_type& t ) : value( t.value ), choice( t.choice ) {}
        };
        ADCONTROLSSHARED_EXPORT bool operator==(const switch_type& lhs, const switch_type& rhs);
        ADCONTROLSSHARED_EXPORT bool operator<(const switch_type& lhs, const switch_type& rhs);
        ADCONTROLSSHARED_EXPORT switch_type tag_invoke( const boost::json::value_to_tag< switch_type >&, const boost::json::value& jv );
        ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const switch_type& );

        // -------------------------------------------
        struct ADCONTROLSSHARED_EXPORT choice_type {
            uint32_t value;
            std::vector< std::string > choice;
            choice_type( uint32_t _1 = 0 ) : value( _1 ) {}
            choice_type( const choice_type& t ) : value( t.value ), choice( t.choice ) {}
        };
        ADCONTROLSSHARED_EXPORT bool operator==(const choice_type& lhs, const choice_type& rhs);
        ADCONTROLSSHARED_EXPORT bool operator<(const choice_type& lhs, const choice_type& rhs);
        ADCONTROLSSHARED_EXPORT choice_type tag_invoke( const boost::json::value_to_tag< choice_type >&, const boost::json::value& jv );
        ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const choice_type& );

        // -------------------------------------------
        struct ADCONTROLSSHARED_EXPORT delay_width_type {
            std::pair<double, double> value;
            delay_width_type( std::pair<double, double>&& _1 = { 0, 1.0e-8 } ) : value( _1 ) {}
            delay_width_type( const delay_width_type& t ) : value( t.value ) {}
        };
        ADCONTROLSSHARED_EXPORT bool operator==(const delay_width_type& lhs, const delay_width_type& rhs);
        ADCONTROLSSHARED_EXPORT bool operator<(const delay_width_type& lhs, const delay_width_type& rhs);
        ADCONTROLSSHARED_EXPORT delay_width_type tag_invoke( const boost::json::value_to_tag< delay_width_type >&, const boost::json::value& jv );
        ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const delay_width_type& );

        // -------------------------------------------
        struct ADCONTROLSSHARED_EXPORT any_type {
            std::string value;   // serialized archive either xml or binary
            boost::uuids::uuid editor_;
            any_type() {}
            any_type( std::string&& _1, const boost::uuids::uuid& editor ) : value( _1 ), editor_( editor ) {}
            any_type( const any_type& t ) : value( t.value ), editor_( t.editor_ ) {}
        };
        ADCONTROLSSHARED_EXPORT bool operator==(const any_type& lhs, const any_type& rhs);
        ADCONTROLSSHARED_EXPORT bool operator<(const any_type& lhs, const any_type& rhs);
        ADCONTROLSSHARED_EXPORT any_type tag_invoke( const boost::json::value_to_tag< any_type >&, const boost::json::value& jv );
        ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const any_type& );

        // -------------------------------------------
        class ADCONTROLSSHARED_EXPORT EventCap {
        public:
            typedef boost::variant< duration_type, voltage_type, switch_type, choice_type, delay_width_type, any_type > value_type;
            typedef std::function< void( const any_type& ) > commit_type;

            EventCap();
            EventCap( const std::string& item_name, const std::string& item_display_name, const value_type& );

            EventCap( const std::string& item_name
                      , const std::string& item_display_name
                      , const value_type& value
                      , std::function< bool( any_type&, commit_type )>&& f
                      , std::function< std::string( const any_type& ) > d = std::function< std::string( const any_type& ) >() );

            EventCap( const EventCap& );

            const std::string& item_name() const;
            const std::string& item_display_name() const;
            const value_type& default_value() const;

            bool edit_any( any_type& a, commit_type f ) const;
            void invalidate_any() const;
            std::string display_value_any( const any_type& a ) const;

            static std::string toString( const value_type& );

        private:
            std::string item_name_;             // id
            std::string item_display_name_;
            value_type default_value_;
            std::function< bool( any_type&, commit_type ) > edit_any_;
            std::function< std::string( const any_type& ) > display_any_;
        };

    }
}
