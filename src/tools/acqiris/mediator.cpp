/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "mediator.hpp"
#if ACQIRIS_DAEMON
# include "daemon.hpp"
#else
# include "document.hpp"
#endif

mediator::mediator()
{
}

mediator::~mediator()
{
}

mediator *
mediator::instance()
{
#if ACQIRIS_DAEMON
    return daemon::instance();
#else
    return document::instance();
#endif
}

void
mediator::prepare_for_run( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p, acqrscontrols::aqdrv4::SubMethodType t )
{
}
    
void
mediator::acqiris_method_adapted( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p )
{
    adapted_method_ = p;
}

void
mediator::eventOut( uint32_t e )
{
}

void
mediator::replyTemperature( int t )
{
}
    
std::shared_ptr< const acqrscontrols::aqdrv4::acqiris_method >
mediator::acqiris_method()
{
    return method_;
}

void
mediator::set_acqiris_method( std::shared_ptr< acqrscontrols::aqdrv4::acqiris_method > p )
{
    method_ = p;
}

