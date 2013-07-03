/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**gnu++0x
**************************************************************************/

#include "tofdeviceid.hpp"
#include "protocolids.hpp"

using namespace tofinterface;

tofDeviceId::tofDeviceId() : protocolId_( Constants::ID00 )
{
}

tofDeviceId::tofDeviceId( const tofDeviceId& t ) : protocolId_( Constants::ID00 )
						 , manufacturer_( t.manufacturer_ )
						 , revision_( t.revision_ )
						 , configurations_( t.configurations_ )
{
}


tofDeviceId::Configuration::Configuration() : configured_( false )
{
}

tofDeviceId::Configuration::Configuration( const tofDeviceId::Configuration& t ) : configured_( t.configured_ )
										 , option_type_( t.option_type_ )
										 , option_name_( t.option_name_ )
{
}

uint32_t
tofDeviceId::protocolId() const
{
    return protocolId_;
}

void
tofDeviceId::manufacturer( const std::string& value )
{
    manufacturer_ = value;
}

const std::string&
tofDeviceId::manufacturer() const
{
    return manufacturer_;
}

void
tofDeviceId::revision( const std::string& value )
{
    revision_ = value;
}

const std::string&
tofDeviceId::revision() const
{
    return revision_;
}

const std::vector< tofDeviceId::Configuration >&
tofDeviceId::configurations() const
{
    return configurations_;
}

std::vector< tofDeviceId::Configuration >&
tofDeviceId::configurations()
{
    return configurations_;
}

///////////

void
tofDeviceId::Configuration::configured( bool f )
{
    configured_ = f;
}

bool
tofDeviceId::Configuration::configured() const
{
    return configured_;
}

void
tofDeviceId::Configuration::option_type( const std::string& value )
{
    option_type_ = value;
}

void
tofDeviceId::Configuration::option_name( const std::string& value )
{
    option_name_ = value;
}

