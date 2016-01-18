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

EventCap::EventCap( const std::string& item_name, const std::string& item_display_name, const value_type& )
{
}

EventCap::EventCap( const EventCap& t ) : item_name_( t.item_name_ )
                                        , item_display_name_( t.item_display_name_ )
                                        , default_value_( t.default_value_ )
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



