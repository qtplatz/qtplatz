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
**
**************************************************************************/

#include "tofmasscommand.hpp"
#include "protocolids.hpp"

using namespace tofinterface;

tofMassCommand::tofMassCommand() : protocolId_( Constants::MCMD )
{
}

tofMassCommand::tofMassCommand( const tofMassCommand& t ) : protocolId_( t.protocolId_ )
{
    std::copy( t.masscommand_.begin(), t.masscommand_.end(), masscommand_.begin() );
    std::copy( t.dcx_.begin(), t.dcx_.end(), dcy_.begin() );
    std::copy( t.dcy_.begin(), t.dcy_.end(), dcy_.begin() );
}

uint32_t
tofMassCommand::protocolId() const
{
    return protocolId_;
}

void
tofMassCommand::masscommand( const boost::array< uint16_t, 65536 >& a )
{
    std::copy( a.begin(), a.end(), &masscommand_[0] );
}

const boost::array< uint16_t, 65536 >&
tofMassCommand::masscommand() const
{
    return masscommand_;
}

void
tofMassCommand::dcx( const boost::array< uint16_t, 65536 >& a )
{
    std::copy( a.begin(), a.end(), dcx_.begin() );
}

const boost::array< uint16_t, 65536 >&
tofMassCommand::dcx() const
{
    return dcx_;
}

void
tofMassCommand::dcy( const boost::array< uint16_t, 65536 >& a )
{
    std::copy( a.begin(), a.end(), dcy_.begin() );
}

const boost::array< uint16_t, 65536 >&
tofMassCommand::dcy() const
{
    return dcy_;
}
