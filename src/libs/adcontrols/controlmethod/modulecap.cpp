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

#include "modulecap.hpp"

using namespace adcontrols::ControlMethod;


ModuleCap::ModuleCap() : clsid_( {0})
{
}

ModuleCap::ModuleCap( const boost::uuids::uuid& clsid
                      , const std::string& model_display_name ) : clsid_( clsid )
                                                                , model_display_name_( model_display_name )
{
}

ModuleCap::ModuleCap( const ModuleCap& t ) : clsid_( t.clsid_ )
                                           , model_display_name_( t.model_display_name_ )
                                           , eventCaps_( t.eventCaps_ )
{
}
    
const boost::uuids::uuid&
ModuleCap::clsid() const
{
    return clsid_;
}

const std::string&
ModuleCap::model_display_name() const
{
    return model_display_name_;
}

const std::vector< EventCap >&
ModuleCap::eventCaps() const
{
    return eventCaps_;
}

std::vector< EventCap >&
ModuleCap::eventCaps()
{
    return eventCaps_;
}
