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

#include "eventcap.hpp"
#include <adportable/float.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace adcontrols::ControlMethod;


EventCap::EventCap()
{
}

EventCap::EventCap( const std::string& item_name
                    , const std::string& item_display_name
                    , const value_type& default_value ) : item_name_( item_name )
                                                        , item_display_name_( item_display_name )
                                                        , default_value_( default_value )
{
}

EventCap::EventCap( const std::string& item_name
                    , const std::string& item_display_name
                    , const value_type& default_value
                    , std::function< bool( any_type&, commit_type )>&& edit
                    , std::function< std::string( const any_type& ) > display ) : item_name_( item_name )
                                                                                , item_display_name_( item_display_name )
                                                                                , default_value_( default_value )
                                                                                , edit_any_( edit )
                                                                                , display_any_( display )
{
}

EventCap::EventCap( const EventCap& t ) : item_name_( t.item_name_ )
                                        , item_display_name_( t.item_display_name_ )
                                        , default_value_( t.default_value_ )
                                        , edit_any_( t.edit_any_ )
                                        , display_any_( t.display_any_ )
{
}

const std::string&
EventCap::item_name() const
{
    return item_name_;
}

const std::string&
EventCap::item_display_name() const
{
    return item_display_name_;
}

const EventCap::value_type&
EventCap::default_value() const
{
    return default_value_;
}

bool
EventCap::edit_any( any_type& a, commit_type signal ) const
{
    if ( edit_any_ )
        return edit_any_( a, signal );
    return false;
}

void
EventCap::invalidate_any() const
{
    if ( edit_any_ ) {
        any_type a;
        edit_any_( a, std::function< void( const any_type& ) >() );
    }
}

std::string
EventCap::display_value_any( const any_type& a ) const
{
    if ( display_any_ )
        return display_any_( a );
    return "";
}

namespace adcontrols {
    namespace ControlMethod {

        struct value_to_string : public boost::static_visitor< std::string > {

            std::string operator()( const voltage_type& t ) const {
                return ( boost::format("%.2f") % t.value ).str();
            };

            std::string operator()( const duration_type& t ) const {
                return ( boost::format("%.3f") % t.value ).str();
            };

            std::string operator()( const switch_type& t ) const {
                return t.value ? t.choice.first : t.choice.second;
            };

            std::string operator()( const choice_type& t ) const {
                if ( t.choice.size() > t.value )
                    return t.choice.at( t.value );
                return "";
            };

            std::string operator()( const delay_width_type& t ) const {
                return ( boost::format("%.3g, %3g") % t.value.first % t.value.second ).str();
            };

            std::string operator()( const any_type& t ) const {
                return ( boost::format("any: editor=%s, size=%d") % t.editor_ % t.value.size() ).str();
            };
        };
    }
}

//static
std::string
EventCap::toString( const value_type& v )
{
    return boost::apply_visitor( value_to_string(), v );
}

namespace adcontrols {  namespace ControlMethod {

        bool operator==(const duration_type& lhs, const duration_type& rhs) {
            return adportable::compare<double>::essentiallyEqual( lhs.value, rhs.value );
        }

        bool operator<(const duration_type& lhs, const duration_type& rhs) {
            return lhs.value < rhs.value;
        }

        bool operator==(const voltage_type& lhs, const voltage_type& rhs) {
            return adportable::compare<double>::essentiallyEqual( lhs.value, rhs.value );
        }

        bool operator<(const voltage_type& lhs, const voltage_type& rhs) {
            return lhs.value < rhs.value;
        }

        bool operator==(const switch_type& lhs, const switch_type& rhs) {
            return lhs.value == rhs.value;
        }

        bool operator<(const switch_type& lhs, const switch_type& rhs) {
            return lhs.value < rhs.value;
        }

        bool operator==(const choice_type& lhs, const choice_type& rhs) {
            return lhs.value == rhs.value;
        }

        bool operator<(const choice_type& lhs, const choice_type& rhs) {
            return lhs.value < rhs.value;
        }

        bool operator==(const delay_width_type& lhs, const delay_width_type& rhs) {
            return lhs.value == rhs.value;
        }

        bool operator<(const delay_width_type& lhs, const delay_width_type& rhs) {
            return lhs.value < rhs.value;
        }

        bool operator==(const any_type& lhs, const any_type& rhs) {
            return lhs.value == rhs.value;
        }

        bool operator<(const any_type& lhs, const any_type& rhs) {
            return lhs.value < rhs.value;
        }

    }
}
