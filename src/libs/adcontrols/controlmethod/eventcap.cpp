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


